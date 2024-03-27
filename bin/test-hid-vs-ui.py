#!/usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#
# Parses all help files (.xhp) to check that hids referencing .ui are up-to-date
# From fdo#67350


import sys
import argparse
import os
import subprocess
import xml.etree.ElementTree as ET
import collections
import re
import smtplib
import email
import email.mime.text
import time
import datetime

# retrieve all hids related to .ui files
def init_hids():
    global args, local_repo
    if local_repo:
        repo_dir = os.path.join(core_repo_dir,'helpcontent2')
        os.chdir(repo_dir)
        return subprocess.check_output(['git','grep','hid="[^"]*/[^"]*">','.'])
    else:
        repo_dir = '/var/tmp/help.git'
        if not os.path.exists(repo_dir):os.makedirs(repo_dir)
        os.chdir(repo_dir)

        if not os.path.exists(os.path.join(repo_dir,'config')):
            subprocess.call(['git','clone','--bare','git://gerrit.libreoffice.org/help',repo_dir])
        elif not args['git_static']:
            subprocess.call(['git','fetch','origin'])
        return subprocess.check_output(['git','grep','hid="[^"]*/[^"]*">','master','--'])

# retrieve .ui files list from the core
def init_core_files():
    global core_repo_dir, local_repo
    core_repo_dir = args['core_repo_dir']
    if core_repo_dir is None:
        core_repo_dir = os.path.dirname(os.path.abspath(os.path.dirname(sys.argv[0])))
        local_repo = True

    if not os.path.exists(core_repo_dir):os.makedirs(core_repo_dir)
    os.chdir(core_repo_dir)

    if not os.path.exists(os.path.join(core_repo_dir,'.git')):
        subprocess.call(['git','clone','git://gerrit.libreoffice.org/core',core_repo_dir])
    elif not args['git_static']:
        subprocess.call(['git','fetch','origin'])
    allfiles = subprocess.check_output(['git','ls-tree','--name-only','--full-name','-r','master'])
    return re.findall(r'.*\.ui',allfiles)


if __name__ == "__main__":

    parser = argparse.ArgumentParser('hid for ui consistency parser')
    parser.add_argument('-s', '--send-to', action='append', help='email address to send the report to. Use one flag per address.', required=False)
    parser.add_argument('-g', '--git-static', action='store_true', help='to avoid contacting remote server to refresh repositories.', required=False)
    parser.add_argument('-r', '--core-repo-dir', help='enforce path to core repository when analyzing .ui files.', required=False)
    args=vars(parser.parse_args())

    uifileslist = init_core_files()  # play it early to gain the local repo identification

    rows = init_hids().splitlines()
    #<tree>:<relative_file>:<text>
    # handled as sets to remove duplicates (and we don't need an iterator)
    targets = collections.defaultdict(set)
    origin = collections.defaultdict(set)

    # fill all matching hids and their parent file
    for row in rows:
        fname, rawtext = row.split(':',1)[0:]
        hid = rawtext.split('hid="')[1].split('"')[0]
        if hid.startswith('.uno'): continue
        uifileraw, compname = hid.rsplit('/',1)
        uifile = uifileraw + ".ui"
        # map modules/ etc, which exist only in install
        # back to their source location
        if uifile.startswith("modules/scalc"):
            uifile = "sc/scalc" + uifile[13:]
        elif uifile.startswith("modules/swriter"):
            uifile = "sw/swriter" + uifile[15:]
        elif uifile.startswith("modules/schart"):
            uifile = "chart2" + uifile[14:]
        elif uifile.startswith("modules/smath"):
            uifile = "starmath/smath" + uifile[13:]
        elif uifile.startswith("modules/sdraw"):
            uifile = "sd/sdraw" + uifile[13:]
        elif uifile.startswith("modules/simpress"):
            uifile = "sd/simpress" + uifile[16:]
        elif uifile.startswith("modules/BasicIDE"):
            uifile = "basctl/basicide" + uifile[16:]
        elif uifile.startswith("modules/sabpilot"):
            uifile = "extensions/sabpilot" + uifile[16:]
        elif uifile.startswith("modules/sbibliography"):
            uifile = "extensions/sbibliography" + uifile[21:]
        elif uifile.startswith("modules/scanner"):
            uifile = "extensions/scanner" + uifile[15:]
        elif uifile.startswith("modules/spropctrlr"):
            uifile = "extensions/spropctrlr" + uifile[18:]
        elif uifile.startswith("sfx"):
            uifile = "sfx2" + uifile[3:]
        elif uifile.startswith("svt"):
            uifile = "svtools" + uifile[3:]
        elif uifile.startswith("fps"):
            uifile = "fpicker" + uifile[3:]
        components = uifile.split('/',1)
        uifile = components[0] + '/uiconfig/' + components[1]
        targets[uifile].add(compname.split(':')[0])
        origin[uifile].add(fname)  # help file(s)

    errors = ''
    # search in all .ui files referenced in help
    # 2 possible errors: file not found in repo, id not found in file
    for uikey in dict.keys(targets):
        if uikey not in uifileslist:
            if len(origin[uikey]) == 1:
                errors += '\nFrom ' + origin[uikey].pop()
            else:
                errors += '\nFrom one of ' + str(origin[uikey]).replace('set(','').replace(')','')
            errors += ', we did not find file '+ uikey+'.'
            continue

        full_path = os.path.join(core_repo_dir,uikey)
        # print full_path
        root = ET.parse(full_path).getroot()
        ids = [element.attrib['id'].split(':')[0] for element in root.findall('.//object[@id]')]
        # print targets[uikey]
        missing_ids = [ element for element in targets[uikey] if element not in ids ]
        if missing_ids:
            if len(origin[uikey]) == 1:
                errors += '\nFrom ' + origin[uikey].pop()
            else:
                errors += '\nFrom one of ' + str(origin[uikey]).replace('set(','').replace(')','')
            errors += ', referenced items '+ str(missing_ids) + ' were not found inside '+ uikey+'.'

    if not errors:
        errors = '\nall is clean\n'

    if args['send_to']:
        msg_from = os.path.basename(sys.argv[0]) + '@libreoffice.org'
        if isinstance(args['send_to'], str):
            msg_to = [args['send_to']]
        else:
            msg_to = args['send_to']
            print("send to array " + msg_to[0])

        server = smtplib.SMTP('localhost')
        body = '''
Hello,

Here is the report for wrong hids from help related to .ui files

'''
        body += errors
        body += '''

Best,

Your friendly LibreOffice Help-ids Checker

Note: The bot generating this message can be found and improved here:
       https://gerrit.libreoffice.org/gitweb?p=dev-tools.git;a=blob;f=scripts/test-hid-vs-ui.py'''
        now = datetime.datetime.now()
        msg = email.mime.text.MIMEText(body, 'plain', 'UTF-8')
        msg['From'] = msg_from
        msg['To'] = msg_to[0]
        msg['Cc'] = ', '.join(msg_to[1:]) # Works only if at least 2 items in tuple
        msg['Date'] = email.utils.formatdate(time.mktime(now.timetuple()))
        msg['Subject'] = 'LibreOffice Gerrit News for python on %s' % (now.date().isoformat())
        msg['Reply-To'] = msg_to[0]
        msg['X-Mailer'] = 'LibreOfficeGerritDigestMailer 1.1'

        server.sendmail(msg_from, msg_to, str(msg))
    else:
        print(errors)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
