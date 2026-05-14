#!/usr/bin/env python3

# Build appx packages

import os
import shutil
import sys
import xml.sax
import argparse
import glob
import getpass
import subprocess
from zipfile import ZipFile, ZIP_DEFLATED, ZIP_STORED

# If we use cygwin, we need a function to convert cygwin paths to Windows paths, to pass them to Win32 apps
try:
    if 'cygwin' not in sys.platform:
        raise SystemError('Not running on cygwin')

    from ctypes import cdll, c_void_p, c_int32, cast, c_wchar_p

    cygwin = cdll.LoadLibrary("cygwin1.dll")
    cygwin_create_path = cygwin.cygwin_create_path
    cygwin_create_path.restype = c_void_p
    cygwin_create_path.argtypes = [c_int32, c_void_p]

    free = cygwin.free
    free.restype = None
    free.argtypes = [c_void_p]

    CCP_POSIX_TO_WIN_W = 1

    def path2win(path):
        result = cygwin_create_path(CCP_POSIX_TO_WIN_W, path.encode('utf-8'))
        if result is None:
            raise Exception("cygwin_create_path failed")
        value = cast(result,c_wchar_p).value
        free(result)
        # Use forward slash, because backslash conflicts e.g. with str.format()
        return value.replace('\\', '/')

except:
    # If we don't use cygwin
    def path2win(path):
        if sys.platform == 'linux':
            # WSL, likely
            return os.popen('wslpath -a -m "' + path + '"').read().replace('\n','')
        # Windows. Use forward slash, because backslash conflicts e.g. with str.format()
        return path.replace('\\', '/')

# Parses core.git filter/source/config/fragments/types/*.xcu.
class FilterTypeHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.name = None
        self.uiname = None
        self.inExtensions = False
        self.inUIName = False
        self.inUINameValue = False
        self.content = []
        self.extensions = None
        self.extensionsSep = " "
    def startElement(self, name, attrs):
        if name == "node":
            for k, v in list(attrs.items()):
                if k == "oor:name":
                    self.name = v
        elif name == "prop":
            for k, v in list(attrs.items()):
                if k == "oor:name" and v == "Extensions":
                    self.inExtensions = True
                if k == "oor:name" and v == "UIName":
                    self.inUIName = True
        elif name == "value" and self.inExtensions:
            for k, v in list(attrs.items()):
                if k == "oor:separator":
                    self.extensionsSep = v
        elif name == "value" and self.inUIName:
            self.inUINameValue = True
    def endElement(self, name):
        if name == "prop" and self.inExtensions:
            self.inExtensions = False
            self.extensions = "".join(self.content).strip().lower().split(self.extensionsSep)
            self.extensionsSep = " "
            self.content = []
        elif name == "prop" and self.inUIName:
            self.inUIName = False
        elif name == "value" and self.inUINameValue:
            self.inUINameValue = False
    def characters(self, content):
        if self.inExtensions:
            self.content.append(content)
        elif self.inUINameValue and content != '':
            self.uiname = content

# Parses core.git filter/source/config/fragments/filters/*.xcu.
class FilterFragmentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.inType = False
        self.typeName = None
        self.inFlags = False
        self.flags = None
        self.inDocumentService = False
        self.documentService = None
        self.content = []
    def startElement(self, name, attrs):
        if name == "prop":
            for k, v in list(attrs.items()):
                if k == "oor:name" and v == "Type":
                    self.inType = True
                elif k == "oor:name" and v == "Flags":
                    self.inFlags = True
                elif k == "oor:name" and v == "DocumentService":
                    self.inDocumentService = True
    def endElement(self, name):
        if name == "prop" and self.inType:
            self.inType = False
            self.typeName = "".join(self.content).strip()
            self.content = []
        elif name == "prop" and self.inFlags:
            self.inFlags = False
            self.flags = "".join(self.content).strip().split(" ")
            self.content = []
        elif name == "prop" and self.inDocumentService:
            self.inDocumentService = False
            self.documentService = "".join(self.content).strip()
            self.content = []
    def characters(self, content):
        if self.inType or self.inFlags or self.inDocumentService:
            self.content.append(content)

# Builds a 'Name' -> ['UIName', 'service', 'flags', ['extension']] dictionary of OWN or IMPORT+EXPORT types.
def getNameExtensions(filterDir):
    # Build a 'Name' -> ['service', 'flags'] dictionary only for OWN or IMPORT+EXPORT
    nameFlags = {}
    filterFragments = os.path.join(filterDir, "filters")
    for filterFragment in os.listdir(filterFragments):
        if not filterFragment.endswith(".xcu"):
            continue
        parser = xml.sax.make_parser()
        handler = FilterFragmentHandler()
        parser.setContentHandler(handler)
        parser.parse(os.path.join(filterFragments, filterFragment))
        if 'OWN' in handler.flags or ("IMPORT" in handler.flags and "EXPORT" in handler.flags):
            nameFlags[handler.typeName] = [handler.documentService, handler.flags]

    # Build resulting dictionary
    nameExtensions = {}
    typeFragments = os.path.join(filterDir, "types")
    for typeFragment in os.listdir(typeFragments):
        if not typeFragment.endswith(".xcu"):
            continue
        parser = xml.sax.make_parser()
        filterTypeHandler = FilterTypeHandler()
        parser.setContentHandler(filterTypeHandler)
        parser.parse(os.path.join(typeFragments, typeFragment))
        # Did we find some extensions? Is it OWN or IMPORT+EXPORT?
        if filterTypeHandler.extensions and filterTypeHandler.name in nameFlags:
            service, flags = nameFlags[filterTypeHandler.name]
            nameExtensions[filterTypeHandler.name] = [filterTypeHandler.uiname, service, flags, filterTypeHandler.extensions]

    return nameExtensions

def normalizeName(dname):
    return dname.lower().translate(str.maketrans(' /', '__'))

def buildExtensions(repo):
    filterDir = os.path.join(repo, "filter/source/config/fragments")
    result = ''
    templateExt = '''                <uap:Extension Category="windows.fileTypeAssociation">
                    <uap:FileTypeAssociation Name="%NAME">
                        <uap:DisplayName>%DNAME</uap:DisplayName>
                        <uap:Logo>Assets/%ICON.png</uap:Logo>
                        <uap:SupportedFileTypes>
                            %FTYPES
                        </uap:SupportedFileTypes>
                    </uap:FileTypeAssociation>
                </uap:Extension>
'''
    templateFT = '<uap:FileType>.%FTYPE</uap:FileType>'
    serviceToIcon = {
        'com.sun.star.text.TextDocument':                 ['text', 'text-template'],
        'com.sun.star.text.GlobalDocument':               ['master-document', 'master-document'],
        'com.sun.star.text.WebDocument':                  ['text', 'text-template'],
        'com.sun.star.sheet.SpreadsheetDocument':         ['spreadsheet', 'spreadsheet-template'],
        'com.sun.star.presentation.PresentationDocument': ['presentation', 'presentation-template'],
        'com.sun.star.drawing.DrawingDocument':           ['drawing', 'drawing-template'],
        'com.sun.star.formula.FormulaProperties':         ['formula', 'formula'],
        'com.sun.star.sdb.OfficeDatabaseDocument':        ['database', 'database'],
    }

    usedSet = {'', '*'} # filter out some wrong extensions from the start
    for name, (uiname, service, flags, extensions) in getNameExtensions(filterDir).items():
        ftXML = ''
        if not uiname:
            uiname = name
        for extension in extensions:
            if extension in usedSet:
                continue
            ftXML += templateFT.replace('%FTYPE', extension)
            usedSet.add(extension)
        if ftXML != '':
            iconMain, iconTemplate = serviceToIcon[service] if service in serviceToIcon else ['soffice', 'soffice']
            icon = iconTemplate if 'TEMPLATEPATH' in flags else iconMain
            result += templateExt.replace('%NAME', normalizeName(name))\
                                 .replace('%DNAME', uiname)\
                                 .replace('%FTYPES', ftXML)\
                                 .replace('%ICON', icon)
    return result

def buildLangResouces(files):
    result = ''
    langTemplate = '        <Resource Language="%BCP47"/>\n'
    for lang in os.listdir(os.path.join(files, 'program/resource')):
        # this is language-neutral resource directory
        if lang == 'common':
            continue
        BCP47 = lang.replace('_', '-').replace('@', '-')
        # despite being mentioned in [MS-LCID] (https://msdn.microsoft.com/en-us/goglobal/bb964664),
        # installer fails to register with error 0x80070057 because these languages are "not valid"
        if BCP47.lower() in ['ast', 'br', 'gd', 'oc', 'sr', 'sr-latin']:
            continue
        result += langTemplate.replace('%BCP47', BCP47)
    return result

def filterPDBs(PDBs, files):
    result = []
    for file in files:
        base, ext = os.path.splitext(file)
        if ext.lower() in ['.dll', '.exe', '.com']:
            pdb = os.path.basename(base) + '.pdb'
            if pdb.lower() in PDBs:
                result.append(pdb)
    return result

def main():
    scriptPath, scriptBaseName = os.path.split(os.path.abspath(__file__))

    try:
        with open(os.path.join(scriptPath, scriptBaseName + '.ini'), encoding='utf8') as input:
            inifile = { name.strip() : value.strip() for name, sep, value in [ line.partition('=') for line in input ] if sep == '=' }
    except FileNotFoundError:
        inifile = {}

    winkit = inifile['winkit'] if 'winkit' in inifile else ''
    makepri_exe = os.path.join(winkit, 'MakePri.exe')
    makeappx_exe = os.path.join(winkit, 'MakeAppx.exe')
    signtool_exe = os.path.join(winkit, 'SignTool.exe')
    distname = inifile['distname'] if 'distname' in inifile else None
    ver = inifile['ver'] if 'ver' in inifile else None
    arch = inifile['arch'] if 'arch' in inifile else None
    files = inifile['files'] if 'files' in inifile else None
    repo = inifile['repo'] if 'repo' in inifile else None
    pfx = inifile['pfx'] if 'pfx' in inifile else None
    builddir = inifile['builddir'] if 'builddir' in inifile else None
    outdir = inifile['outdir'] if 'outdir' in inifile else os.path.join(os.getcwd(), 'out')

    parser = argparse.ArgumentParser()
    group = parser.add_argument_group('main arguments')
    group.add_argument('-coda', action='store_true', help='use if packaging CODA')
    group.add_argument('-nopdb', action='store_true', help='do not package .pdb files')
    group.add_argument('-distname', default=distname, required=(distname is None), help='product name' + ((', defaults to ' + distname) if distname else ''))
    group.add_argument('-ver', default=ver, required=(ver is None), help='product version' + ((', defaults to ' + ver) if ver else ''))
    group.add_argument('-arch', default=arch, required=(arch is None), help='product architecture: x64 or x86' + ((', defaults to ' + arch) if arch else ''))
    group.add_argument('-files', default=files, required=(files is None), help='path to install files created by --with-package-format=installed' + ((', defaults to ' + files) if files else ''))
    group.add_argument('-repo', default=repo, required=(repo is None), help='path to core.git source code repository' + ((', defaults to ' + repo) if repo else ''))
    group.add_argument('-pfx', default=pfx, help='path to optional sign key (.pfx) to use for signing' + ((', defaults to ' + pfx) if pfx else ''))
    group.add_argument('-password', default='', help='password for pfx')
    group.add_argument('-builddir', default=builddir, help='path to core.git build directory; used for collecting PDBs, defaults to ' + (pfx if pfx else 'source code repository path'))
    group.add_argument('-outdir', default=outdir, help='where the package is to be generated, defaults to ' + outdir)
    args = parser.parse_args()
    if not args.builddir:
        args.builddir = args.repo

    workdir = os.path.join(args.outdir, 'tmp')
    if os.path.exists(workdir):
        shutil.rmtree(workdir)
    os.makedirs(workdir)

    assetsrootdir = os.path.join(workdir, 'assetsroot')
    assetsdir = os.path.join(assetsrootdir, 'Assets')
    os.makedirs(assetsdir)
    origassetsdir = os.path.join(scriptPath, 'Assets')
    for f in os.listdir(origassetsdir):
        shutil.copy(os.path.join(origassetsdir, f), assetsdir)
    pri_file = os.path.join(workdir, 'resources.pri')

    # patch bootstrap.ini
    bootstrap_file = os.path.join(workdir, 'bootstrap.ini')
    with open(os.path.join(args.files , 'program/bootstrap.ini'), encoding='utf8') as input:
        with open(bootstrap_file, mode='w', encoding='utf8') as output:
            patched = False
            for line in input:
                if line.startswith('UserInstallation='):
                    line = 'UserInstallation=$SYSUSERCONFIG/' + args.distname + '/appx\n'
                    patched = True
                output.write(line)
            if not patched:
                raise Exception("Failed to patch bootstrap.ini")

    # build file associations, and append office URI scheme protocols
    extensions = buildExtensions(args.repo) + '''                <uap:Extension Category="windows.protocol">
                    <uap:Protocol Name="ms-excel"/>
                </uap:Extension>
                <uap:Extension Category="windows.protocol">
                    <uap:Protocol Name="ms-powerpoint"/>
                </uap:Extension>
                <uap:Extension Category="windows.protocol">
                    <uap:Protocol Name="ms-visio"/>
                </uap:Extension>
                <uap:Extension Category="windows.protocol">
                    <uap:Protocol Name="ms-word"/>
                </uap:Extension>
                <uap:Extension Category="windows.protocol">
                    <uap:Protocol Name="vnd.libreoffice.command"/>
                </uap:Extension>
'''

    # build language resource list
    resources = buildLangResouces(args.files)

    # to use PackagingLayout, we must build the package. (Also potentially allows to build
    # several APPX for multiple platforms.)
    # we need separate manifest files for package and individual APPX, with neutral arch for
    # package, and defined arch for APPX. Also only store extensions for individual APPX.
    with open(os.path.join(scriptPath, 'AppxManifest.xml.in'), encoding='utf8') as input:
        manifest_xml = input.read().replace('%VER', args.ver).replace('%RES', resources)
    with open(os.path.join(workdir, 'AppxManifest.xml'), mode='w', encoding='utf8') as output:
        output.write(manifest_xml.replace('%ARCH', 'neutral').replace('%EXT', ''))
    manifest_xml = manifest_xml.replace('%EXT', extensions)
    with open(os.path.join(workdir, 'AppxManifest.' + args.arch + '.xml'), mode='w', encoding='utf8') as output:
        output.write(manifest_xml.replace('%ARCH', args.arch))

    packaginglayout_xml = os.path.join(workdir, 'PackagingLayout.xml')
    with open(os.path.join(scriptPath, 'PackagingLayout.xml.in'), encoding='utf8') as input:
        with open(packaginglayout_xml, mode='w', encoding='utf8') as output:
            output.write(input.read().replace('%WORKDIR', path2win(workdir))\
                                     .replace('%ASSETS', path2win(assetsdir))\
                                     .replace('%FILES', path2win(args.files))\
                                     .replace('%DISTNAME', args.distname)\
                                     .replace('%VER', args.ver)\
                                     .replace('%ARCH', args.arch)\
                                     .replace('%BOOTSTRAP', path2win(bootstrap_file))\
                                     .replace('%PRI', path2win(pri_file)))

    # create PRI
    subprocess.run([makepri_exe, 'new', '/pr', path2win(assetsrootdir), '/cf', path2win(os.path.join(scriptPath, 'MakePri.xml')), '/in', 'CollaboraProductivityLtd.CollaboraOfficeDesktop', '/of', path2win(pri_file)]).check_returncode()

    fileBaseName = args.distname + '.' + args.ver + '.' + args.arch + '.'

    # now build the package. This depends on MakeAppx.exe being in the $PATH if no winkit in inifile
    subprocess.run([makeappx_exe, 'build', '/f', path2win(packaginglayout_xml), '/op', path2win(workdir)]).check_returncode()

    # MakeAppx.exe picks the output extension from the manifest: a manifest
    # referencing 10.0.17763+ schema (com / desktop2 namespaces, or that
    # MinVersion) is written as .msix / .msixbundle; an older schema yields
    # .appx / .appxbundle. Same container in either case; pick whichever
    # filename MakeAppx actually produced.
    pkgExt = 'msix' if os.path.exists(os.path.join(workdir, fileBaseName + 'msix')) else 'appx'
    appxName = os.path.join(workdir, fileBaseName + pkgExt)
    appxsymName = os.path.join(workdir, fileBaseName + pkgExt + 'sym')
    appxuploadName = os.path.join(args.outdir, fileBaseName + pkgExt + 'upload')

    if args.pfx:
        pfxWin = path2win(args.pfx)
        # check password of the PFX
        password = args.password
        runResult = subprocess.run(['CertUtil.exe', '/p', password, '-dump', pfxWin], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if runResult.returncode != 0:
            if password == '': # only ask if no password was in the command line
                password = getpass.getpass(prompt='Enter password for ' + args.pfx + ': ')
        # sign it! This depends on SignTool.exe being in the $PATH if no winkit in inifile
        subprocess.run([signtool_exe, 'sign', '/fd', 'SHA256', '/a', '/f', pfxWin, '/p', password, path2win(appxName)]).check_returncode()
    else:
        print('No pfx provided; the package will be unsigned.')

    if not args.nopdb:
        # collect PDBs with public symbols: https://stackoverflow.com/questions/44510991/how-to-create-appxsym-for-zipping-appxupload
        # depends on PDBCopy.exe in $PATH - https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
        # well - let's try to go without PDBCopy, simply copy the PDBs as is: installing WDK breaks our configure
        print('Collecting PDBs...')
        linkTarget = os.path.join(args.builddir, 'workdir/LinkTarget')
        with ZipFile(appxsymName, 'w') as myzip:
            filelist = glob.glob(os.path.join(args.files, '**/*.*'), recursive=True)
            for target in ['Executable', 'Library']:
                pdbPath = os.path.join(linkTarget, target)
                for pdb in filterPDBs([f.lower() for f in os.listdir(pdbPath) if f.lower().endswith('.pdb')], filelist):
                    pdbFile = os.path.join(pdbPath, pdb)
                    # print('adding ' + pdbFile)
                    myzip.write(pdbFile, pdb, ZIP_DEFLATED)
                    #os.system('pdbcopy.exe "{0}" "{1}" -p'.format(pathOld, pathNew))

    # create appxupload
    print('Creating appxupload...')
    with ZipFile(appxuploadName, 'w') as myzip:
        if not args.nopdb:
            members = [appxName, appxsymName]
        else:
            members = [appxName]
        for file in members:
            basename = os.path.basename(file)
            # print('adding ' + basename)
            myzip.write(file, basename, ZIP_STORED)

    # cleanup
    shutil.rmtree(workdir)

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
