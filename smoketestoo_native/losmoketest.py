# Version: MPL 1.1 / GPLv3+ / LGPLv3+

# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Yifan Jiang, SUSE <yfjiang@suse.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# Portions created by the Ted are Copyright (C) 2010 Ted. All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
##
# The tool is designed to enable test machine fully automatically run smoketest
# with both daily and pre release build located in dev-build.libreoffice.org.
#
# The tool is named as losmoketest for its purpose, meanwhile it help you to
# check, download and install the latest build. By the fact the installation is
# designed not to be different from manually doing these repeated work, the
# installed libreoffice build can also be good for manual test.

import sys, os, platform
import datetime, time
import subprocess, shutil
import glob, re
import urllib, urllib2
import logging, getopt
try:
    import ConfigParser as configparser # Python 3.0 change
except ImportError:
    import configparser

build_version = "3.4"
tag_version = "3-4"

build_check_interval = 5  #seconds

# Distro list
RPM_DISTRO_LIST = ['SuSE', 'fedora', 'redhat', 'centos', 'mandrake', 'mandriva', 'yellowdog', 'turbolinux']
DEB_DISTRO_LIST = ['debian', 'ubuntu', 'Ubuntu']

# Server urls
SERVER_URL = "http://dev-builds.libreoffice.org"

# Local dirs
root_dir = os.getcwd()
DOWNLOAD_DIR = os.path.join(root_dir, "_download")
USR_DIR = os.path.join(root_dir, "_libo_smoke_user")
LOCAL_BUILD_INFO_FILE = "build.cfg"

ROOT_DIR_LIB = os.path.join(root_dir, 'lib')
ROOT_DIR_LIB32 = os.path.join(root_dir, 'lib32')
ROOT_DIR_LIB64 = os.path.join(root_dir, 'lib64')
ROOT_DIR_BIN32 = os.path.join(root_dir, 'bin32')
ROOT_DIR_BIN64 = os.path.join(root_dir, 'bin64')

CPPUNITTESTER = os.path.join(root_dir, 'cppunittester')

# INSTALL_DIR = os.path.join(root_dir, "_libo_smoke_installation")
INSTALL_DIR = "" # Installation dir

# SOFFICE_BIN bin
if platform.system() == "Linux":
    SOFFICE_BIN= INSTALL_DIR + os.sep + "opt" + os.sep + "libreoffice" + build_version + os.sep + "program" + os.sep + "soffice"

# Relative build url
## pre-releases
PR_RPM_X86_PATH = "pre-releases/rpm/x86/"
PR_RPM_X86_64_PATH = "pre-releases/rpm/x86_64/"

PR_DEB_X86_PATH = "pre-releases/deb/x86/"
PR_DEB_X86_64_PATH = "pre-releases/deb/x86_64/"

PR_MAC_X86_PATH = "pre-releases/mac/x86/"
PR_MAC_PPC_PATH = "pre-releases/mac/ppc/"

PR_WIN_X86_PATH = "pre-releases/win/x86/"

## daily_master
DAILY_MASTER_RPM_X86_PATH =    "daily/Linux_x86_Release_Configuration/master/current"
DAILY_MASTER_RPM_X86_64_PATH = "daily/Linux_x86_64_Release_Configuration/master/current"

DAILY_MASTER_DEB_X86_PATH =    "daily/Linux_x86_Release_Configuration/master/current"
DAILY_MASTER_DEB_X86_64_PATH = "daily/Linux_x86_64_Release_Configuration/master/current" # No build yet

DAILY_MASTER_MAC_X86_PATH =    "daily/MacOSX_10.6.7_Intel_no-moz/master/current"
DAILY_MASTER_MAC_PPC_PATH =    "^$" # No build yet

DAILY_MASTER_WIN_X86_PATH =    "daily/Windows_Release_Configuration/master/current"

## daily_branch
DAILY_BRANCH_RPM_X86_PATH =    "daily/Linux_x86_Release_Configuration/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_RPM_X86_64_PATH = "daily/Linux_x86_64_Release_Configuration/libreoffice-" + tag_version + "/current"

DAILY_BRANCH_DEB_X86_PATH =    "daily/Linux_x86_Release_Configuration/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_DEB_X86_64_PATH = "daily/Linux_x86_64_Release_Configuration/libreoffice-" + tag_version + "/current"

DAILY_BRANCH_MAC_X86_PATH =    "daily/MacOSX_10.6.7_Intel_no-moz/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_MAC_PPC_PATH =    "^$" # No build yet

DAILY_BRANCH_WIN_X86_PATH =    "daily/Windows_Release_Configuration/libreoffice-" + tag_version + "/current"



def platform_info():

    s = platform.system()
    arch_name = platform.machine()

    if arch_name in ['x86', 'i386', 'i586', 'i686']:
        arch_name = 'x86'

    if s == "Linux":
        if platform.dist()[0] in RPM_DISTRO_LIST:
            distro_name = platform.dist()[0]
            pck_name = 'rpm'
        elif platform.dist()[0] in DEB_DISTRO_LIST:
            distro_name = platform.dist()[0]
            pck_name = 'deb'
    elif s == "Windows":
        distro_name = platform.dist()[0]
        pck_name = 'exe'
    else:
        distro_name = platform.dist()[0]
        pck_name = 'dmg'

    return distro_name, pck_name, arch_name

def local_build_info(t):

    config = configparser.RawConfigParser()
    config.read(LOCAL_BUILD_INFO_FILE)

    try:
        build_name = config.get(t, 'build_name').strip('\n')
        build_time = datetime.datetime.strptime(config.get(t, 'build_time').strip('\n'), '%d-%b-%Y %H:%M')
    except ValueError:
        build_name = ''
        build_time = datetime.datetime.min

    return build_name, build_time

def get_url_regexp(t, package, arch):
    '''
    return a url containing download links, i.e:

        http://dev-builds.libreoffice.org/pre-releases/rpm/x86_64/
        http://dev-builds.libreoffice.org/daily/Windows_Release_Configuration/libreoffice-3-4/current/
        http://dev-builds.libreoffice.org/daily/Linux_x86_Release_Configuration/libreoffice-3-4/current/

    meanwhile return a regexp object that matching corresponding downloadable
    package and its timestamp '''

    url = ""
    reg = re.compile('^$')
    pck = package
    arc = arch

    if t == 'pre-releases':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + PR_RPM_X86_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*x86_install-rpm.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + PR_RPM_X86_64_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*x86-64_install-rpm.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + PR_DEB_X86_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*x86_install-deb.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + PR_DEB_X86_64_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*x86-64_install-deb.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + PR_WIN_X86_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*Win_x86_install_multi.exe)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + PR_MAC_X86_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*MacOS_x86_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + PR_MAC_PPC_PATH
            reg = re.compile('\<a\ href=\"(LibO_\d.*MacOS_PPC_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        else:
            logger.error("Unable to handle the system or arch!")
    elif t == 'daily_master':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_RPM_X86_PATH
            reg = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install-rpm_en-US.tar.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_MASTER_RPM_X86_64_PATH
            reg = re.compile('^$') # No build yet
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_DEB_X86_PATH
            reg = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install-deb_en-US.tar.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_MASTER_DEB_X86_64_PATH
            reg = re.compile('^$') # No build yet
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_WIN_X86_PATH
            reg = re.compile('^$') # No build yet
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_MAC_X86_PATH
            reg = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + DAILY_MASTER_MAC_PPC_PATH
            reg = re.compile('^$') # No build yet
        else:
            logger.error("Unable to handle the system or arch!")
    elif t == 'daily_branch':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_RPM_X86_PATH
            reg = re.compile('\<a\ href=\"(.*LibO_.*x86_install-rpm_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_BRANCH_RPM_X86_64_PATH
            reg = re.compile('\<a\ href=\"(.*LibO_.*x86-64_install-rpm_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_DEB_X86_PATH
            reg = re.compile('\<a\ href=\"(.*LibO_.*x86_install-deb_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_BRANCH_DEB_X86_64_PATH
            reg = re.compile('\<a\ href=\"(.*LibO_.*x86-64_install-deb_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_WIN_X86_PATH
            reg = re.compile('\<a\ href=\"(.*LibO_.*install_.*\.exe)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_MAC_X86_PATH
            reg = re.compile('^$') # No build yet
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + DAILY_BRANCH_MAC_PPC_PATH
            reg = re.compile('^$') # No build yet
        else:
            logger.error("Unable to handle the system or arch!")
    else:
        logger.error("Error build type! The build type has to be:\n pre-releases, daily_master, daily_branch")

    return url, reg

def remote_build_info(url_reg):
    ''' Get the latest proper build info (build_name, build_time) from
    url. '''

    p = platform_info()
    pck = p[1]
    arc = p[2]
    r = url_reg[1]

    f = urllib2.urlopen(url_reg[0])
    c = ''.join(f.readlines())
    f.close()

    build_list = r.findall(c)

    build_name = ''
    build_time = datetime.datetime.min

    for b in build_list:
        if datetime.datetime.strptime(b[1] + ' ' + b[2], '%d-%b-%Y %H:%M') > build_time:
            build_name = b[0]
            build_time = datetime.datetime.strptime(b[1] + ' ' + b[2], '%d-%b-%Y %H:%M')

    return build_name, build_time


def fetch_build(url, filename):
    ''' Download a build from address url/filename '''

    logger.info("Downloading ... " + filename)

    u = urllib2.urlopen(url + '/' + filename)

    try:
        f = open(DOWNLOAD_DIR + '/' + filename, 'wb')
        f.write(u.read())
    finally:
        f.close()

    return True

def set_build_config(section, option, value):

    config = configparser.RawConfigParser()
    config.readfp(open(LOCAL_BUILD_INFO_FILE))
    config.set(section, option, value)
    with open(LOCAL_BUILD_INFO_FILE, 'wb') as cfgfile:
        config.write(cfgfile)

def uninstall():
    ''' Kill libreoffice processes and uninstall all previously installed
    libreoffice packages '''

    logger.info("Uninstalling ...")

    pck = platform_info()[1]

    if pck == 'rpm':
        cmd_query  = ["rpm", "-qa"]
        cmd_filter = ["grep", "-e", "libreoffice"+build_version, "-e", "libobasis"+build_version]

        P_query  = subprocess.Popen(cmd_query, stdout = subprocess.PIPE)
        P_filter = subprocess.Popen(cmd_filter, stdin = P_query.stdout, stdout = subprocess.PIPE)
        P_query.stdout.close() # protection when P_filter exit before P_query
        str_filter = P_filter.communicate()[0]

        if str_filter == "":
            logger.warning("Nothing to uninstall")
            return
        else:
            cmd = ["sudo", "rpm", "-e"] + str_filter.split()
    elif pck == 'deb':
        cmd_query = ["dpkg", "--get-selections", "libreoffice"+build_version+"*", "libobasis"+build_version+"*"]
        cmd_filter = ["cut", "-f", "1"]

        P_query = subprocess.Popen(cmd_query, stdout = subprocess.PIPE)
        P_filter = subprocess.Popen(cmd_filter, stdin = P_query.stdout, stdout = subprocess.PIPE)
        P_query.stdout.close()
        str_filter = P_filter.communicate()[0]

        if str_filter == "":
            logger.warning("Nothing to uninstall")
            return
        else:
            cmd = ["sudo", "dpkg ", "-P"] + str_filter.split()
    elif pck == 'exe':
        pass
    elif pck == 'dmg':
        pass
    else:
        logger.warning("Non supported package system")

    subprocess.check_call(cmd)

def init_testing():
    logger.info("Initializing ...")

    post_testing()

    if not os.path.exists(DOWNLOAD_DIR):
        os.mkdir(DOWNLOAD_DIR)

    if not os.path.exists(USR_DIR):
        os.mkdir(USR_DIR)

    # set up links
    try:
        if platform.system() == "Linux" and platform_info()[2] == "x86":
            os.symlink(ROOT_DIR_LIB32, ROOT_DIR_LIB)
            os.symlink(os.path.join(ROOT_DIR_BIN32, 'cppunittester'), CPPUNITTESTER)
        elif platform.system() == "Linux" and platform_info()[2] == "x86_64":
            os.symlink(ROOT_DIR_LIB64, ROOT_DIR_LIB)
            os.symlink(os.path.join(ROOT_DIR_BIN64, 'cppunittester'), CPPUNITTESTER)
        elif platform.system() == "Windows" and platform_info()[2] == "x86":
            pass
        else:
            pass
    except OSError:
        pass

def post_testing():
    logger.info("Cleaning up ...")

    # clean up the extracted installer dir
    for r in os.walk(DOWNLOAD_DIR):
        if r[0] == DOWNLOAD_DIR:
            for d in r[1]:
                shutil.rmtree(os.path.join(r[0], d))

def install(filename):
    ''' filename: local file path of tar.gz, dmg or exe. The script will
    extract the package and then install it '''

    logger.info("Installing ... " + filename)

    def _is_not_filtered(s):
        ''' True if the package s is not intended to installed.  '''
        filter_pattern_list = ['.*kde.*', '.*gnome.*', '.*desktop.*', '!.*\.rpm$', '!.*\.deb$']
        for p in filter_pattern_list:
            r = re.compile(p)
            if r.match(s):
                return False
        return True

    fn, ext = os.path.splitext(filename)
    pcklist = []

    if ext == '.exe':
        # extract
        installer_dir = os.path.join(DOWNLOAD_DIR, filename.strip(ext))
        subprocess.check_call([filename, '/EXTRACTONLY=ON', '/S', '/D='+installer_dir])
        # install
        installer = glob.glob(os.path.join(installer_dir, 'libreoffice*msi'))[0]
        subprocess.check_call(['msiexec', '-i', installer, '-passive', 'ADDLOCAL=all'])
    elif ext == '.dmg':
        return
    elif ext == '.gz':
        # extract
        subprocess.check_call(['tar', 'xzf', filename, '-C', DOWNLOAD_DIR])

        # get a filtered install list
        for root, dirs, files in os.walk(DOWNLOAD_DIR):
            if 'RPMS' in root or 'DEBS' in root:
                pcklist = pcklist + [os.path.join(root_dir, root, f) for f in files]
        pcklist = filter(_is_not_filtered, pcklist)

        # install
        if platform_info()[1] == 'rpm':
            install_cmd = ["sudo", "rpm", "-iv"] + pcklist
        elif platform_info()[1] == 'deb':
            install_cmd = ["sudo", "dpkg", "-i"] + pcklist
        else:
            logger.error('Cannot generate install command')
            return

        subprocess.check_call(install_cmd)

    else:
        logger.info("Unrecognized file extension")

def verify_smoketest():
    logger.info("Testing ...")

    s = platform.system()
    p = platform_info()
    pck = p[1]
    arc = p[2]

    if s == "Linux":
        os.environ['LD_LIBRARY_PATH'] = ""
        os.environ['LD_LIBRARY_PATH'] = ':'.join([os.path.join(root_dir, 'lib'), INSTALL_DIR + '/opt/libreoffice' + build_version + '/ure/lib', os.environ['LD_LIBRARY_PATH']])
        cmd_cppu = [
                    CPPUNITTESTER,
                    "-env:UNO_SERVICES=file://"+ INSTALL_DIR +"/opt/libreoffice" + build_version + "/ure/share/misc/services.rdb",
                    "-env:UNO_TYPES=" + os.path.join(os.path.join(root_dir, 'lib'), "types.rdb"),
                    "-env:arg-soffice=path:" + SOFFICE_BIN,
                    "-env:arg-user=" + USR_DIR,
                    "-env:arg-env=" + os.environ['LD_LIBRARY_PATH'],
                    "-env:arg-testarg.smoketest.doc=" + os.path.join(root_dir, "doc/smoketestdoc.sxw"),
                    "--protector",
                    os.path.join(root_dir, "lib/unoexceptionprotector.so"), "unoexceptionprotector",
                    os.path.join(root_dir, "lib/libsmoketest.so")
                   ]
    else:
        logger.warning('The smoketest does not support this platform yet!')

    subprocess.check_call(cmd_cppu)

def usage():

    print "\n[Usage]\n\n  -f Force testing without asking \n\
  -t Testing type pre-release/daily \n\
  -v Run smoketest verification directly \n\
  -i Install the latest build in the DOWNLOAD directory \n\
  -u Uninstall any existed libreoffice build \n\
"

def main():

    interactive = True
    build_type = "pre-releases"

    package_type = platform_info()[1]
    arch_type = platform_info()[2]

    interactive = True
    build_type = "pre-releases"

    # Handling options and arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "uihfvt:", ["uninstall", "install", "help", "force", "verify", "type="])
    except getopt.GetoptError, err:
        logger.error(str(err))
        usage()
        sys.exit(2)

    for o, a in opts:
        if ("-t" in o) or ("--type" in o):
            build_type = a

    for o, a in opts:
        if o in ("-f", "--force"):
            interactive = False
        elif o in ("-t", "--type"):
            pass
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-v", "--verify"):
            init_testing()
            verify_smoketest()
            sys.exit()
        elif o in ("-i", "--install"):
            init_testing()
            install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[0])
            sys.exit()
        elif o in ("-u", "--uninstall"):
            uninstall()
            sys.exit()
        else:
            assert False, "Unhandled option"

    if interactive == True:
        key = raw_input("The testing will OVERRIDE existed libreoffice, continue(y/N)? ")
        if not (key == "y" or key == "Y" or key == "yes"):
            sys.exit()

    url_reg = get_url_regexp(build_type, package_type, arch_type)

    init_testing()
    while True:
        logger.info('Checking new build ...')
        try:
            remote_build = remote_build_info(url_reg)
            local_build = local_build_info(build_type)

            if remote_build[1] > local_build[1]:
                logger.info('Found New build: ' + remote_build[0])
                if fetch_build(url_reg[0], remote_build[0]):
                    set_build_config(build_type, 'build_name', remote_build[0])
                    set_build_config(build_type, 'build_time', datetime.datetime.strftime(remote_build[1], '%d-%b-%Y %H:%M'))
                else:
                    logger.error('Download build failed!')
                    continue
                uninstall()
                install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[0])
                verify_smoketest()
            else:
                time.sleep(build_check_interval)
                continue
        except urllib2.URLError, HTTPError:
            logger.error('Error fetch remote build info.')
            time.sleep(build_check_interval)
            continue
        except KeyboardInterrupt:
            sys.exit()
        except:
            time.sleep(build_check_interval)
            continue

def ut():
    verify_smoketest()

if __name__ == '__main__':

    # logging
    logger = logging.getLogger('')
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler('losmoketest.log')
    ch = logging.StreamHandler()

    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)

    logger.addHandler(ch)
    logger.addHandler(fh)

    main()
    # ut()
