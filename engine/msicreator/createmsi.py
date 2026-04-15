#!/usr/bin/env python3

# Copyright 2017-2018 Jussi Pakkanen et al
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import subprocess
import uuid
import json
from glob import glob
import platform
import xml.etree.ElementTree as ET

sys.path.append(os.getcwd())

def gen_guid():
    return str(uuid.uuid4()).upper()

class Node:
    def __init__(self, dirs, files):
        assert(isinstance(dirs, list))
        assert(isinstance(files, list))
        self.dirs = dirs
        self.files = files

class UIGraphics:
    def __init__(self):
        self.banner = None
        self.background = None

class PackageGenerator:

    def __init__(self, jsonfile):
        with open(jsonfile, 'rb') as f:
            jsondata = json.load(f)
            self.product_name = jsondata['product_name']
            self.manufacturer = jsondata['manufacturer']
            self.version = jsondata['version']
            self.comments = jsondata['comments']
            self.installdir = jsondata['installdir']
            self.license_file = jsondata.get('license_file', None)
            self.name = jsondata['name']
            self.guid = jsondata.get('product_guid', '*')
            self.upgrade_guid = jsondata['upgrade_guid']
            self.basename = jsondata['name_base']
            self.need_msvcrt = jsondata.get('need_msvcrt', False)
            self.addremove_icon = jsondata.get('addremove_icon', None)
            self.startmenu_shortcut = jsondata.get('startmenu_shortcut', None)
            self.desktop_shortcut = jsondata.get('desktop_shortcut', None)
            self.main_xml = self.basename + '.wxs'
            self.main_o = self.basename + '.wixobj'
            self.idnum = 0
            self.graphics = UIGraphics()
            if 'graphics' in jsondata:
                if 'banner' in jsondata['graphics']:
                    self.graphics.banner = jsondata['graphics']['banner']
                if 'background' in jsondata['graphics']:
                    self.graphics.background = jsondata['graphics']['background']
            if 'arch' in jsondata:
                self.arch = jsondata['arch']
            else:
                # rely on the environment variable since python architecture may not be the same as system architecture
                if 'PROGRAMFILES(X86)' in os.environ:
                    self.arch = 64
                else:
                    self.arch = 32 if '32' in platform.architecture()[0] else 64
            self.final_output = '%s-%s-%d.msi' % (self.basename, self.version, self.arch)
            if self.arch == 64:
                self.progfile_dir = 'ProgramFiles64Folder'
                if platform.system() == "Windows":
                    redist_glob = 'C:\\Program Files\\Microsoft Visual Studio\\*\\*\\VC\\Redist\\MSVC\\v*\\MergeModules\\Microsoft_VC*_CRT_x64.msm'
                else:
                    redist_glob = '/usr/share/msicreator/Microsoft_VC141_CRT_x64.msm'
            else:
                self.progfile_dir = 'ProgramFilesFolder'
                if platform.system() == "Windows":
                    redist_glob = 'C:\\Program Files\\Microsoft Visual Studio\\*\\Community\\VC\\Redist\\MSVC\\*\\MergeModules\\Microsoft_VC*_CRT_x86.msm'
                else:
                    redist_glob = '/usr/share/msicreator/Microsoft_VC141_CRT_x86.msm'
            trials = glob(redist_glob)
            if self.need_msvcrt:
                if len(trials) > 1:
                    sys.exit('There are more than one redist dirs: ' +
                             ', '.join(trials))
                if len(trials) == 0:
                    sys.exit('No redist dirs were detected, install MSM redistributables with VS installer.')
                self.redist_path = trials[0]
            self.component_num = 0
            self.registry_entries = jsondata.get('registry_entries', None)
            self.major_upgrade = jsondata.get('major_upgrade', None)
            self.parts = jsondata['parts']
            self.feature_components = {}
            self.feature_properties = {}

    def generate_files(self):
        self.root = ET.Element('Wix', {'xmlns': 'http://schemas.microsoft.com/wix/2006/wi'})
        product = ET.SubElement(self.root, 'Product', {
            'Name': self.product_name,
            'Manufacturer': self.manufacturer,
            'Id': self.guid,
            'UpgradeCode': self.upgrade_guid,
            'Language': '1033',
            'Codepage':  '1252',
            'Version': self.version,
        })

        package = ET.SubElement(product, 'Package',  {
            'Id': '*',
            'Keywords': 'Installer',
            'Description': '%s %s installer' % (self.name, self.version),
            'Comments': self.comments,
            'Manufacturer': self.manufacturer,
            'InstallerVersion': '500',
            'Languages': '1033',
            'Compressed': 'yes',
            'SummaryCodepage': '1252',
        })

        if self.major_upgrade is not None:
            majorupgrade = ET.SubElement(product, 'MajorUpgrade', {})
            for mkey in self.major_upgrade.keys():
                majorupgrade.set(mkey, self.major_upgrade[mkey])
        else:
            ET.SubElement(product, 'MajorUpgrade', {'DowngradeErrorMessage': 'A newer version of %s is already installed.' % self.name})
        if self.arch == 64:
            package.set('Platform', 'x64')
        ET.SubElement(product, 'Media', {
            'Id': '1',
            'Cabinet': self.basename + '.cab',
            'EmbedCab': 'yes',
        })
        targetdir = ET.SubElement(product, 'Directory', {
            'Id': 'TARGETDIR',
            'Name': 'SourceDir',
        })
        progfiledir = ET.SubElement(targetdir, 'Directory', {
            'Id': self.progfile_dir,
        })
        pmf = ET.SubElement(targetdir, 'Directory', {'Id': 'ProgramMenuFolder'},)
        if self.startmenu_shortcut is not None:
            ET.SubElement(pmf, 'Directory', {
                'Id': 'ApplicationProgramsFolder',
                'Name': self.product_name,
            })
        if self.desktop_shortcut is not None:
            ET.SubElement(pmf, 'Directory', {'Id': 'DesktopFolder',
                                             'Name': 'Desktop',
            })
        installdir = ET.SubElement(progfiledir, 'Directory', {
            'Id': 'INSTALLDIR',
            'Name': self.installdir,
        })
        if self.need_msvcrt:
            ET.SubElement(installdir, 'Merge', {
                'Id': 'VCRedist',
                'SourceFile': self.redist_path,
                'DiskId': '1',
                'Language': '0',
            })

        if self.startmenu_shortcut is not None:
            ap = ET.SubElement(product, 'DirectoryRef', {'Id': 'ApplicationProgramsFolder'})
            comp = ET.SubElement(ap, 'Component', {'Id': 'ApplicationShortcut',
                                                   'Guid': gen_guid(),
                                                   })
            ET.SubElement(comp, 'Shortcut', {'Id': 'ApplicationStartMenuShortcut',
                                             'Name': self.product_name,
                                             'Description': self.comments,
                                             'Target': '[INSTALLDIR]' + self.startmenu_shortcut,
                                             'WorkingDirectory': 'INSTALLDIR',
            })
            ET.SubElement(comp, 'RemoveFolder', {'Id': 'RemoveApplicationProgramsFolder',
                                                 'Directory': 'ApplicationProgramsFolder',
                                                 'On': 'uninstall',
                                                 })
            ET.SubElement(comp, 'RegistryValue', {'Root': 'HKCU',
                                                  'Key': 'Software\\Microsoft\\' + self.name,
                                                  'Name': 'Installed',
                                                  'Type': 'integer',
                                                  'Value': '1',
                                                  'KeyPath': 'yes',
                                                  })
        if self.desktop_shortcut is not None:
            desk = ET.SubElement(product, 'DirectoryRef', {'Id': 'DesktopFolder'})
            comp = ET.SubElement(desk, 'Component', {'Id':'ApplicationShortcutDesktop',
                                                     'Guid': gen_guid(),
                                                     })
            ET.SubElement(comp, 'Shortcut', {'Id': 'ApplicationDesktopShortcut',
                                             'Name': self.product_name,
                                             'Description': self.comments,
                                             'Target': '[INSTALLDIR]' + self.desktop_shortcut,
                                             'WorkingDirectory': 'INSTALLDIR',
            })
            ET.SubElement(comp, 'RemoveFolder', {'Id': 'RemoveDesktopFolder',
                                                 'Directory': 'DesktopFolder',
                                                 'On': 'uninstall',
                                                 })
            ET.SubElement(comp, 'RegistryValue', {'Root': 'HKCU',
                                                  'Key': 'Software\\Microsoft\\' + self.name,
                                                  'Name': 'Installed',
                                                  'Type': 'integer',
                                                  'Value': '1',
                                                  'KeyPath': 'yes',
                                                  })

        ET.SubElement(product, 'Property', {
            'Id': 'WIXUI_INSTALLDIR',
            'Value': 'INSTALLDIR',
        })
        if platform.system() == "Windows":
            if self.license_file:
                ET.SubElement(product, 'UIRef', {
                    'Id': 'WixUI_FeatureTree',
                })
            else:
                self.create_licenseless_dialog_entries(product)

        if self.graphics.banner is not None:
            ET.SubElement(product, 'WixVariable', {
                'Id': 'WixUIBannerBmp',
                'Value': self.graphics.banner,
            })
        if self.graphics.background is not None:
            ET.SubElement(product, 'WixVariable', {
                'Id': 'WixUIDialogBmp',
                'Value': self.graphics.background,
            })

        top_feature = ET.SubElement(product, 'Feature', {
            'Id': 'Complete',
            'Title': self.name + ' ' + self.version,
            'Description': 'The complete package',
            'Display': 'expand',
            'Level': '1',
            'ConfigurableDirectory': 'INSTALLDIR',
        })

        for f in self.parts:
            self.scan_feature(top_feature, installdir, 1, f)

        if self.need_msvcrt:
            vcredist_feature = ET.SubElement(top_feature, 'Feature', {
                'Id': 'VCRedist',
                'Title': 'Visual C++ runtime',
                'AllowAdvertise': 'no',
                'Display': 'hidden',
                'Level': '1',
            })
            ET.SubElement(vcredist_feature, 'MergeRef', {'Id': 'VCRedist'})
        if self.startmenu_shortcut is not None:
            ET.SubElement(top_feature, 'ComponentRef', {'Id': 'ApplicationShortcut'})
        if self.desktop_shortcut is not None:
            ET.SubElement(top_feature, 'ComponentRef', {'Id': 'ApplicationShortcutDesktop'})
        if self.addremove_icon is not None:
            icoid = 'addremoveicon.ico'
            ET.SubElement(product, 'Icon', {'Id': icoid,
                                            'SourceFile': self.addremove_icon,
            })
            ET.SubElement(product, 'Property', {'Id': 'ARPPRODUCTICON',
                                                'Value': icoid,
            })

        if self.registry_entries is not None:
            registry_entries_directory = ET.SubElement(product, 'DirectoryRef', {'Id': 'TARGETDIR'})
            registry_entries_component = ET.SubElement(registry_entries_directory, 'Component', {'Id': 'RegistryEntries', 'Guid': gen_guid()})
            if self.arch == 64:
                registry_entries_component.set('Win64', 'yes')
            ET.SubElement(top_feature, 'ComponentRef', {'Id': 'RegistryEntries'})
            for r in self.registry_entries:
                self.create_registry_entries(registry_entries_component, r)

        ET.ElementTree(self.root).write(self.main_xml, encoding='utf-8', xml_declaration=True)
        # ElementTree can not do prettyprinting so do it manually
        import xml.dom.minidom
        doc = xml.dom.minidom.parse(self.main_xml)
        with open(self.main_xml, 'w') as of:
            of.write(doc.toprettyxml(indent=' '))

    def create_registry_entries(self, comp, reg):
        reg_key = ET.SubElement(comp, 'RegistryKey', {
            'Root': reg['root'],
            'Key': reg['key'],
            'Action': reg['action'],
        })
        ET.SubElement(reg_key, 'RegistryValue', {
            'Name': reg['name'],
            'Type': reg['type'],
            'Value': reg['value'],
            'KeyPath': reg['key_path'],
          })

    def scan_feature(self, top_feature, installdir, depth, feature):
        for sd in [feature['staged_dir']]:
            if '/' in sd or '\\' in sd:
                sys.exit('Staged_dir %s must not have a path segment.' % sd)
            nodes = {}
            for root, dirs, files in os.walk(sd):
                cur_node = Node(dirs, files)
                nodes[root] = cur_node
            fdict = {
                'Id': feature['id'],
                'Title': feature['title'],
                'Description': feature['description'],
                'Level': '1'
            }
            if feature.get('absent', 'ab') == 'disallow':
                fdict['Absent'] = 'disallow'
            self.feature_properties[sd] = fdict

            self.feature_components[sd] = []
            self.create_xml(nodes, sd, installdir, sd)
            self.build_features(nodes, top_feature, sd)

    def build_features(self, nodes, top_feature, staging_dir):
        feature = ET.SubElement(top_feature, 'Feature',  self.feature_properties[staging_dir])
        for component_id in self.feature_components[staging_dir]:
            ET.SubElement(feature, 'ComponentRef', {
                'Id': component_id,
            })

    def path_to_id(self, pathname):
        #return re.sub(r'[^a-zA-Z0-9_.]', '_', str(pathname))[-72:]
        idstr = f'pathid{self.idnum}'
        self.idnum += 1
        return idstr

    def create_xml(self, nodes, current_dir, parent_xml_node, staging_dir):
        cur_node = nodes[current_dir]
        if cur_node.files:
            component_id = 'ApplicationFiles%d' % self.component_num
            comp_xml_node = ET.SubElement(parent_xml_node, 'Component', {
                'Id': component_id,
                'Guid': gen_guid(),
            })
            self.feature_components[staging_dir].append(component_id)
            if self.arch == 64:
                comp_xml_node.set('Win64', 'yes')
            if platform.system() == "Windows" and self.component_num == 0:
                ET.SubElement(comp_xml_node, 'Environment', {
                    'Id': 'Environment',
                    'Name': 'PATH',
                    'Part': 'last',
                    'System': 'yes',
                    'Action': 'set',
                    'Value': '[INSTALLDIR]',
                })
            self.component_num += 1
            for f in cur_node.files:
                file_id = self.path_to_id(os.path.join(current_dir, f))
                ET.SubElement(comp_xml_node, 'File', {
                    'Id': file_id,
                    'Name': f,
                    'Source': os.path.join(current_dir, f),
                })

        for dirname in cur_node.dirs:
            dir_id = self.path_to_id(os.path.join(current_dir, dirname))
            dir_node = ET.SubElement(parent_xml_node, 'Directory', {
                'Id': dir_id,
                'Name': dirname,
            })
            self.create_xml(nodes, os.path.join(current_dir, dirname), dir_node, staging_dir)

    def create_licenseless_dialog_entries(self, product_element):
        ui = ET.SubElement(product_element, 'UI', {
            'Id': 'WixUI_FeatureTree'
        })

        ET.SubElement(ui, 'TextStyle', {
            'Id': 'WixUI_Font_Normal',
            'FaceName': 'Tahoma',
            'Size': '8'
        })

        ET.SubElement(ui, 'TextStyle', {
            'Id': 'WixUI_Font_Bigger',
            'FaceName': 'Tahoma',
            'Size': '12'
        })

        ET.SubElement(ui, 'TextStyle', {
            'Id': 'WixUI_Font_Title',
            'FaceName': 'Tahoma',
            'Size': '9',
            'Bold': 'yes'
        })

        ET.SubElement(ui, 'Property', {
            'Id': 'DefaultUIFont',
            'Value': 'WixUI_Font_Normal'
        })

        ET.SubElement(ui, 'Property', {
            'Id': 'WixUI_Mode',
            'Value': 'FeatureTree'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'ErrorDlg'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'FatalError'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'FilesInUse'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'MsiRMFilesInUse'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'PrepareDlg'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'ProgressDlg'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'ResumeDlg'
        })

        ET.SubElement(ui, 'DialogRef', {
            'Id': 'UserExit'
        })

        pub_exit = ET.SubElement(ui, 'Publish', {
            'Dialog': 'ExitDialog',
            'Control': 'Finish',
            'Event': 'EndDialog',
            'Value': 'Return',
            'Order': '999'
        })

        pub_exit.text = '1'

        pub_welcome_next = ET.SubElement(ui, 'Publish', {
            'Dialog': 'WelcomeDlg',
            'Control': 'Next',
            'Event': 'NewDialog',
            'Value': 'CustomizeDlg'
        })

        pub_welcome_next.text = 'NOT Installed'

        pub_welcome_maint_next = ET.SubElement(ui, 'Publish', {
            'Dialog': 'WelcomeDlg',
            'Control': 'Next',
            'Event': 'NewDialog',
            'Value': 'VerifyReadyDlg'
        })

        pub_welcome_maint_next.text = 'Installed AND PATCH'

        pub_customize_back_maint = ET.SubElement(ui, 'Publish', {
            'Dialog': 'CustomizeDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'MaintenanceTypeDlg',
            'Order': '1'
        })

        pub_customize_back_maint.text = 'Installed'

        pub_customize_back_welcome = ET.SubElement(ui, 'Publish', {
            'Dialog': 'CustomizeDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'WelcomeDlg',
            'Order': '2'
        })

        pub_customize_back_welcome.text = 'Not Installed'

        pub_customize_next = ET.SubElement(ui, 'Publish', {
            'Dialog': 'CustomizeDlg',
            'Control': 'Next',
            'Event': 'NewDialog',
            'Value': 'VerifyReadyDlg'
        })

        pub_customize_next.text = '1'

        pub_verify_customize_back = ET.SubElement(ui, 'Publish', {
            'Dialog': 'VerifyReadyDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'CustomizeDlg',
            'Order': '1'
        })

        pub_verify_customize_back.text = 'NOT Installed OR WixUI_InstallMode = "Change"'

        pub_verify_maint_back = ET.SubElement(ui, 'Publish', {
            'Dialog': 'VerifyReadyDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'MaintenanceTypeDlg',
            'Order': '2'
        })

        pub_verify_maint_back.text = 'Installed AND NOT PATCH'

        pub_verify_welcome_back = ET.SubElement(ui, 'Publish', {
            'Dialog': 'VerifyReadyDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'WelcomeDlg',
            'Order': '3'
        })

        pub_verify_welcome_back.text = 'Installed AND PATCH'

        pub_maint_welcome_next = ET.SubElement(ui, 'Publish', {
            'Dialog': 'MaintenanceWelcomeDlg',
            'Control': 'Next',
            'Event': 'NewDialog',
            'Value': 'MaintenanceTypeDlg'
        })

        pub_maint_welcome_next.text = '1'

        pub_maint_type_change = ET.SubElement(ui, 'Publish', {
            'Dialog': 'MaintenanceTypeDlg',
            'Control': 'ChangeButton',
            'Event': 'NewDialog',
            'Value': 'CustomizeDlg'
        })

        pub_maint_type_change.text = '1'

        pub_maint_type_repair = ET.SubElement(ui, 'Publish', {
            'Dialog': 'MaintenanceTypeDlg',
            'Control': 'RepairButton',
            'Event': 'NewDialog',
            'Value': 'VerifyReadyDlg'
        })

        pub_maint_type_repair.text = '1'

        pub_maint_type_remove = ET.SubElement(ui, 'Publish', {
            'Dialog': 'MaintenanceTypeDlg',
            'Control': 'RemoveButton',
            'Event': 'NewDialog',
            'Value': 'VerifyReadyDlg'
        })

        pub_maint_type_remove.text = '1'

        pub_maint_type_back = ET.SubElement(ui, 'Publish', {
            'Dialog': 'MaintenanceTypeDlg',
            'Control': 'Back',
            'Event': 'NewDialog',
            'Value': 'MaintenanceWelcomeDlg'
        })

        pub_maint_type_back.text = '1'

        ET.SubElement(product_element, 'UIRef', {
            'Id': 'WixUI_Common',
        })

    def build_package(self):
        wixdir = 'c:\\Program Files (x86)\\Wix Toolset v3.11\\bin'
        if platform.system() != "Windows":
            wixdir = '/usr/bin'
        if not os.path.isdir(wixdir):
            wixdir = 'c:\\Program Files (x86)\\Wix Toolset v3.11\\bin'
        if not os.path.isdir(wixdir):
            print("ERROR: This script requires WIX")
            sys.exit(1)
        if platform.system() == "Windows":
            subprocess.check_call([os.path.join(wixdir, 'candle'), self.main_xml])
            subprocess.check_call([os.path.join(wixdir, 'light'),
                                   '-ext', 'WixUIExtension',
                                   '-cultures:en-us',
                                   '-dWixUILicenseRtf=' + self.license_file if self.license_file else '',
                                   '-dcl:high',
                                   '-out', self.final_output,
                                   self.main_o])
        else:
            subprocess.check_call([os.path.join(wixdir, 'wixl'), '-o', self.final_output, self.main_xml])

def run(args):
    if len(args) != 1:
        sys.exit('createmsi.py <msi definition json>')
    jsonfile = args[0]
    if '/' in jsonfile or '\\' in jsonfile:
        sys.exit('Input file %s must not contain a path segment.' % jsonfile)
    p = PackageGenerator(jsonfile)
    p.generate_files()
    p.build_package()

if __name__ == '__main__':
    run(sys.argv[1:])
