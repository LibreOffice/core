#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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

# FIXME: make this configurable via options or autodetect it
build_version = "3.5"
tag_version = "3-5"
# devel build
branding_pack="lodev"
basis_pack="lodevbasis"
# stable build
#branding_pack="libreoffice"
#basis_pack="libobasis"

# possible program files of libreoffice, put all platform paths are
# expected in this list
lo_all_paths = [
    "/opt/lodev" + build_version, \
    "/opt/libreoffice" + build_version, \
    "/usr/lib/libreoffice", \
    "/usr/lib64/libreoffice", \
    "C:\program file\libreoffice", \
    ]

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
LOCAL_BUILD_INFO_FILE = os.path.join(root_dir, "build.cfg")

# INSTALL_DIR = os.path.join(root_dir, "_libo_smoke_installation")
INSTALL_DIR = "" # Installation dir

# SOFFICE_BIN bin
if platform.system() == "Linux":
    SOFFICE_BIN = "soffice"
    LOSMOKETEST_BIN = "losmoketest"
elif platform.system() == "Windows":
    SOFFICE_BIN = "soffice.exe"
    LOSMOKETEST_BIN = "losmoketest"
else:
    SOFFICE_BIN = "soffice"
    LOSMOKETEST_BIN = "losmoketest"

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
DAILY_MASTER_RPM_X86_PATH =    "daily/Linux-x86_10-Release_Configuration/master/current"
DAILY_MASTER_RPM_X86_64_PATH = "daily/Linux-x86_64_11-Release_Configuration/master/current"

DAILY_MASTER_DEB_X86_PATH =    "daily/Linux-x86_10-Release_Configuration/master/current"
DAILY_MASTER_DEB_X86_64_PATH = "daily/Linux-x86_64_11-Release_Configuration/master/current"

DAILY_MASTER_MAC_X86_PATH =    "daily/MacOSX-Intel@3-OSX_10.6.0-gcc_4.0.1/master/current"
DAILY_MASTER_MAC_PPC_PATH =    "daily/MacOSX-PPC@12-OSX_10.5.0-gcc_4.0.1/master/current" # No build yet

DAILY_MASTER_WIN_X86_PATH =    "daily/Win-x86@7-MinGW/master/current" # cross compling build

## daily_branch
DAILY_BRANCH_RPM_X86_PATH =    "daily/Linux-x86_10-Release_Configuration/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_RPM_X86_64_PATH = "daily/Linux-x86_64_11-Release_Configuration/libreoffice-" + tag_version + "/current"

DAILY_BRANCH_DEB_X86_PATH =    "daily/Linux-x86_10-Release_Configuration/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_DEB_X86_64_PATH = "daily/Linux-x86_64_11-Release_Configuration/libreoffice-" + tag_version + "/current"

DAILY_BRANCH_MAC_X86_PATH =    "daily/MacOSX-Intel@3-OSX_10.6.0-gcc_4.0.1/libreoffice-" + tag_version + "/current"
DAILY_BRANCH_MAC_PPC_PATH =    "daily/MacOSX-PPC@12-OSX_10.5.0-gcc_4.0.1/libreoffice-" + tag_version + "/current"

DAILY_BRANCH_WIN_X86_PATH =    "daily/Win-x86@7-MinGW/libreoffice-" + tag_version + "/current"



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

    if not os.path.exists(LOCAL_BUILD_INFO_FILE):
        logger.error("Can't find the file: " + LOCAL_BUILD_INFO_FILE)
        sys.exit()

    config = configparser.RawConfigParser()
    config.read(LOCAL_BUILD_INFO_FILE)

    try:
        build_name = config.get(t, 'build_name').strip('\n')
        build_time = datetime.datetime.strptime(config.get(t, 'build_time').strip('\n'), '%d-%b-%Y %H:%M')
    except ValueError:
        build_name = ''
        build_time = datetime.datetime.min

    try:
        testpack_name = config.get(t, 'testpack_name').strip('\n')
        testpack_build_time = datetime.datetime.strptime(config.get(t, 'testpack_build_time').strip('\n'), '%d-%b-%Y %H:%M')
    except ValueError:
        testpack_name = ''
        testpack_build_time = datetime.datetime.min

    return build_name, build_time, testpack_name, testpack_build_time

def get_url_regexp(t, package, arch):
    '''
    return a url containing download links, i.e:

        http://dev-builds.libreoffice.org/pre-releases/rpm/x86_64/
        http://dev-builds.libreoffice.org/daily/Windows_Release_Configuration/libreoffice-3-4/current/
        http://dev-builds.libreoffice.org/daily/Linux_x86_Release_Configuration/libreoffice-3-4/current/

    meanwhile return a regexp object that matching corresponding downloadable
    package and its timestamp '''

    url = ""
    reg_lo = re.compile('^$')
    reg_tst = re.compile('^$')
    pck = package
    arc = arch

    if t == 'pre-releases':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + PR_RPM_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*x86_install-rpm.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
            reg_tst = re.compile('\<a\ href=\"(LibO-Test.*.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + PR_RPM_X86_64_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*x86-64_install-rpm.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
            reg_tst = re.compile('\<a\ href=\"(LibO-Test.*.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + PR_DEB_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*x86_install-deb.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + PR_DEB_X86_64_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*x86-64_install-deb.*en-US.*\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + PR_WIN_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*Win_x86_install_multi.exe)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + PR_MAC_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*MacOS_x86_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + PR_MAC_PPC_PATH
            reg_lo = re.compile('\<a\ href=\"(LibO_\d.*MacOS_PPC_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        else:
            logger.error("Unable to handle the system or arch!")
    elif t == 'daily_master':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_RPM_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install-rpm_en-US.tar.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_MASTER_RPM_X86_64_PATH
            reg_lo = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86-64_install-rpm_en-US.tar.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_DEB_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install-deb_en-US.tar.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_MASTER_DEB_X86_64_PATH
            reg_lo = re.compile('^$') # No build yet
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_WIN_X86_PATH
            reg_lo = re.compile('^$') # No build yet
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_MASTER_MAC_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(master\~\d.*LibO-Dev_.*x86_install_en-US.dmg)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + DAILY_MASTER_MAC_PPC_PATH
            reg_lo = re.compile('^$') # No build yet
        else:
            logger.error("Unable to handle the system or arch!")
    elif t == 'daily_branch':
        if pck == "rpm" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_RPM_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(.*LibO_.*x86_install-rpm_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "rpm" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_BRANCH_RPM_X86_64_PATH
            reg_lo = re.compile('\<a\ href=\"(.*LibO_.*x86-64_install-rpm_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_DEB_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(.*LibO_.*x86_install-deb_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "deb" and arc == "x86_64":
            url = SERVER_URL + "/" + DAILY_BRANCH_DEB_X86_64_PATH
            reg_lo = re.compile('\<a\ href=\"(.*LibO_.*x86-64_install-deb_en-US\.tar\.gz)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "exe" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_WIN_X86_PATH
            reg_lo = re.compile('\<a\ href=\"(.*LibO_.*install_.*\.exe)\".*(\d{2}\-[a-zA-Z]{3}\-\d{4}).*(\d{2}:\d{2}).*')
        elif pck == "dmg" and arc == "x86":
            url = SERVER_URL + "/" + DAILY_BRANCH_MAC_X86_PATH
            reg_lo = re.compile('^$') # No build yet
        elif pck == "dmg" and arc == "ppc":
            url = SERVER_URL + "/" + DAILY_BRANCH_MAC_PPC_PATH
            reg_lo = re.compile('^$') # No build yet
        else:
            logger.error("Unable to handle the system or arch!")
    else:
        logger.error("Error build type! The build type has to be:\n pre-releases, daily_master, daily_branch")

    return url, reg_lo, reg_tst

def remote_build_info(url_reg):
    ''' Get the latest proper build info (build_name, build_time) from
    url. '''

    p = platform_info()
    pck = p[1]
    arc = p[2]
    r = url_reg[1]
    r_t = url_reg[2]

    f = urllib2.urlopen(url_reg[0])
    c = ''.join(f.readlines())
    f.close()

    build_list = r.findall(c)
    test_list = r_t.findall(c)

    build_name = ''
    build_time = datetime.datetime.min
    testpack_build_time = datetime.datetime.min
    testpack_name = ''


    for b in build_list:
        if datetime.datetime.strptime(b[1] + ' ' + b[2], '%d-%b-%Y %H:%M') > build_time:
            build_name = b[0]
            try:
                build_time = datetime.datetime.strptime(b[1] + ' ' + b[2], '%d-%b-%Y %H:%M')
            except:
                print "remote_build_info: wrong time date&format"

    for t in test_list:
        if datetime.datetime.strptime(t[1] + ' ' + t[2], '%d-%b-%Y %H:%M') > testpack_build_time:
            testpack_name = t[0]
            try:
                testpack_build_time = datetime.datetime.strptime(t[1] + ' ' + t[2], '%d-%b-%Y %H:%M')
            except:
                print "remote_build_info: wrong time date&format"

    return build_name, build_time, testpack_name, testpack_build_time

# return True when something was downloaded
def download(url_reg, build_type):
    logger.info('Checking new build ...')

    try:
        remote_build = remote_build_info(url_reg)
        local_build = local_build_info(build_type)

        if remote_build[1] > local_build[1]:
            logger.info('Found New LO build: ' + remote_build[0])
            if fetch_build(url_reg[0], remote_build[0]):
                set_build_config(build_type, 'build_name', remote_build[0])
                set_build_config(build_type, 'build_time', datetime.datetime.strftime(remote_build[1], '%d-%b-%Y %H:%M'))
            else:
                logger.error('Download libreoffice build failed!')

        if remote_build[3] > local_build[3] and (remote_build[1] - remote_build[3]) < datetime.timedelta(hours=1):
            logger.info('Found a relevant smoketest package: ' + remote_build[2])
            if fetch_build(url_reg[0], remote_build[2]):
                set_build_config(build_type, 'testpack_name', remote_build[2])
                set_build_config(build_type, 'testpack_build_time', datetime.datetime.strftime(remote_build[3], '%d-%b-%Y %H:%M'))
                return True
            else:
                logger.warning("Failed to find corresponding smoketest package")

    except urllib2.URLError, HTTPError:
        logger.error('Error fetch remote build info.')
        return False
    except KeyboardInterrupt:
        sys.exit()
    except:
        logger.error('Error fetch remote build info.')
        return False

    return False

def fetch_build(url, filename):
    ''' Download a build from address url/filename '''

    logger.info("Downloading ... " + filename)

    u = urllib2.urlopen(url + '/' + filename)

    try:
        f = open(DOWNLOAD_DIR + '/' + filename, 'wb')
        f.write(u.read())
        f.close()
    except urllib2.HTTPError, e:
        print "HTTP Error:",e.code , url
    except urllib2.URLError, e:
        print "URL Error:",e.reason , url

    return True

def set_build_config(section, option, value):

    config = configparser.RawConfigParser()
    config.readfp(open(LOCAL_BUILD_INFO_FILE))
    config.set(section, option, value)
    with open(LOCAL_BUILD_INFO_FILE, 'wb') as cfgfile:
        config.write(cfgfile)

def uninstall(build_type):
    ''' Kill libreoffice processes and uninstall all previously installed
    libreoffice packages '''

    if build_type == "pre-releases":
        branding_pack="libreoffice"
        basis_pack="libobasis"

    logger.info("Uninstalling ...")

    pck = platform_info()[1]

    if pck == 'rpm':
        cmd_query  = ["rpm", "-qa"]
        cmd_filter = ["grep", \
                      "-e", branding_pack+build_version, \
                      "-e", basis_pack+build_version,  \
                      ]

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
        cmd_query = ["dpkg", "--get-selections", branding_pack+build_version+"*", basis_pack+build_version+"*"]
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

    if not os.path.exists(LOCAL_BUILD_INFO_FILE):

        init_build_cfg = '[daily_branch]' + os.linesep\
                       + 'build_name =' + os.linesep\
                       + 'build_time =' + os.linesep\
                       + 'testpack_name =' + os.linesep\
                       + 'testpack_build_time =' + os.linesep\
                       + '[daily_master]' + os.linesep\
                       + 'build_name =' + os.linesep\
                       + 'build_time =' + os.linesep\
                       + 'testpack_name =' + os.linesep\
                       + 'testpack_build_time =' + os.linesep\
                       + '[pre-releases]' + os.linesep\
                       + 'build_name =' + os.linesep\
                       + 'build_time =' + os.linesep \
                       + 'testpack_name =' + os.linesep\
                       + 'testpack_build_time =' + os.linesep

        with open(LOCAL_BUILD_INFO_FILE, 'w+') as f:
            f.write(init_build_cfg)
        f.close()

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
        install_pcklist = filter(_is_not_filtered, pcklist)

        # install
        if platform_info()[1] == 'rpm':
            install_cmd = ["sudo", "rpm", "-iv"] + install_pcklist
            clean_tmp_cmd = ["sudo", "rm", "-f"] + pcklist
        elif platform_info()[1] == 'deb':
            install_cmd = ["sudo", "dpkg", "-i"] + install_pcklist
        else:
            logger.error('Cannot generate install command')
            return

        subprocess.check_call(install_cmd)
        subprocess.check_call(clean_tmp_cmd)

    else:
        logger.info("Unrecognized file extension")

def verify_smoketest(headless):
    logger.info("Testing ...")

    s = platform.system()
    p = platform_info()
    pck = p[1]
    arc = p[2]

    lo_testable_paths = filter(lambda p: \
                               os.path.exists(p + os.sep + "program" + os.sep + LOSMOKETEST_BIN) and \
                               os.path.exists(p + os.sep + "program" + os.sep + SOFFICE_BIN), \
                               lo_all_paths)

    if not lo_testable_paths:
        logger.error("Not found any Libreoffice or Test packages!")
        sys.exit(1)
    else:
        cmd_smoketests = [ p + os.sep + "program" + os.sep + LOSMOKETEST_BIN for p in lo_testable_paths ]

    if len(lo_testable_paths) > 1:
        logger.info("++More than one testable build is found, test them one by one.")

    # subprocess.call(cmd_smoketest);
    for c in cmd_smoketests:
        pattern = re.compile(LOSMOKETEST_BIN + "$")
        logger.info("  Test Binary: " + pattern.sub(SOFFICE_BIN, c))
        subprocess.call(c)

def usage():

    print "\n[Usage]\n\n  -f Force testing without asking \n\
  -t Testing type pre-release/daily \n\
  -l Download and test last builds in a loop \n\
  -v Run smoketest verification directly \n\
  -s Use the headless mode when running the tests \n\
  -i Install the latest build in the DOWNLOAD directory \n\
  -u Uninstall any existed libreoffice build \n\
  -d Download the latest build for the given test type \n\
"

def main():

    interactive = True
    build_type = "pre-releases"

    package_type = platform_info()[1]
    arch_type = platform_info()[2]

    loop = False
    interactive = True
    headless = False
    build_type = "pre-releases"

    # Handling options and arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "dluihfvst:", ["download", "loop", "uninstall", "install", "help", "force", "verify", "headless", "type="])
    except getopt.GetoptError, err:
        logger.error(str(err))
        usage()
        sys.exit(2)

    for o, a in opts:
        if ("-t" in o) or ("--type" in o):
            build_type = a
        elif o in ("-s", "--headless"):
            headless = True

    url_reg = get_url_regexp(build_type, package_type, arch_type)

    for o, a in opts:
        if o in ("-f", "--force"):
            interactive = False
        elif o in ("-t", "--type"):
            pass
        elif o in ("-s", "--headless"):
            pass
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-v", "--verify"):
            init_testing()
            verify_smoketest(headless)
            sys.exit()
        elif o in ("-i", "--install"):
            init_testing()
            uninstall(build_type)
            install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[0])
            install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[2])
            sys.exit()
        elif o in ("-u", "--uninstall"):
            uninstall(build_type)
            sys.exit()
        elif o in ("-d", "--download"):
            init_testing()
            download(url_reg, build_type)
            sys.exit()
        elif o in ("-l", "--loop"):
            loop = True
        else:
            assert False, "Unhandled option: " + o

    if interactive == True:
        key = raw_input("The testing will OVERRIDE existed libreoffice, continue(y/N)? ")
        if not (key == "y" or key == "Y" or key == "yes"):
            sys.exit()

    init_testing()
    first_run = True
    while loop or first_run:
        if download(url_reg, build_type):
            try:
                # FIXME: uninstall script fails but it need not break the whole game; so try it twice
                try:
                    uninstall(build_type)
                except:
                    logger.error("Some errors happened during uninstall. Trying once again.")
                    uninstall(build_type)

                install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[0])
                install(DOWNLOAD_DIR + os.sep + local_build_info(build_type)[2])
                verify_smoketest(headless)

            except KeyboardInterrupt:
                sys.exit()
            except:
                continue
        else:
             logger.warning("No new build found.")
        if loop:
            time.sleep(build_check_interval)

        first_run = False


if __name__ == '__main__':

    # logging
    logger = logging.getLogger('')
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler(os.path.basename(__file__) + '.log')
    ch = logging.StreamHandler()

    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)

    logger.addHandler(ch)
    logger.addHandler(fh)

    main()
