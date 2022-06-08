#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import configparser
import hashlib
import magic
import os
import requests
import shutil
import sys
import tempfile

from bs4 import BeautifulSoup
from attachment_mimetypes import mimetypes
from concurrent.futures import ThreadPoolExecutor, as_completed
from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry

forums = {
    # https://wiki.documentfoundation.org/Website/Web_Sites_services#Unofficial_and_Related_Pages
    'en': "https://forum.openoffice.org/en/forum",
    'es': "https://forum.openoffice.org/es/forum",
    'fr': "https://forum.openoffice.org/fr/forum",
    'hu': "https://forum.openoffice.org/hu/forum",
    'it': "https://forum.openoffice.org/it/forum",
    'ja': "https://forum.openoffice.org/ja/forum",
    'nl': "https://forum.openoffice.org/nl/forum",
    'pl': "https://forum.openoffice.org/pl/forum",
    'vi': "https://forum.openoffice.org/vi/forum",
    'tr': "https://forum.libreoffice.org.tr",
    'de': "https://www.openoffice-forum.de",
    'de2': "https://www.libreoffice-forum.de",
    'de3': "https://de.openoffice.info",
    # Others
    'mso-en': "https://www.msofficeforums.com",
    'mso-de': "https://www.ms-office-forum.net/forum",
}

def do_login(session, url, configFile):
    config = configparser.ConfigParser()

    config.read(configFile)
    username = config.get('login', 'username')
    password = config.get('login', 'password')
    resp = session.post(url + '/login.php?do=login', {
    'vb_login_username':        username,
    'vb_login_password':        '',
    'vb_login_md5password':     hashlib.md5(password.encode()).hexdigest(),
    'vb_login_md5password_utf': hashlib.md5(password.encode()).hexdigest(),
    'cookieuser': 1,
    'do': 'login',
    's': '',
    'securitytoken': 'guest'
    })

    if resp.status_code != 200:
        return False

    soup = BeautifulSoup(resp.content, 'lxml')
    for p in soup.find_all("p"):
        if 'Thank you for logging in' in p.get_text():
            return True
        elif 'Danke für Ihre Anmeldung' in p.get_text():
            return True

    return False

def get_attachments_from_url(lang, url, pathes):

    print("Checking " + url)

    startIndex = 0

    # Keep the index and resume from there
    indexFile = os.path.join(pathes.outdir, lang + ".index")
    if os.path.isfile(indexFile):
        with open(indexFile) as f:
            startIndex = int(f.readline().rstrip()) + 1
    else:
        if lang == 'hu':
            startIndex = 1300

    session = requests.Session()
    retry = Retry(connect=3, backoff_factor=0.5)
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)

    if lang.startswith("mso"):
        if not do_login(session, url, pathes.config):
            print("Can't log in to " + url)
            return

    invalidCount = 0
    for i in range(startIndex, 999999):
        if lang.startswith("mso"):
            fileUrl = url + "/attachment.php?attachmentid=" + str(i)
        else:
            fileUrl = url + "/download/file.php?id=" + str(i)

        h = session.head(fileUrl)
        header = h.headers
        content_type = header.get('content-type')
        if "html" in content_type:
            # Let's assume this is an invalid file link
            invalidCount += 1

            # Let's assume, if we get 100 invalid files, that there are no more files
            if invalidCount == 100:
                print("No more attachments found in " + url)
                break
        else:
            invalidCount = 0

            r = session.get(fileUrl, allow_redirects=True)
            with tempfile.NamedTemporaryFile() as tmp:
                tmp.write(r.content)
                mimetype = magic.from_file(tmp.name, mime=True)
                if mimetype in mimetypes:
                    suffix = mimetypes[mimetype]
                    suffixDir = os.path.join(pathes.outdir, suffix)
                    try:
                        os.mkdir(suffixDir)
                    except:
                        pass

                    download = os.path.join(suffixDir,
                            "forum-" + lang + '-' + str(i) + '.' + suffix)

                    print("Downloading as " + download)
                    shutil.copy(tmp.name, download)

            # Save the index
            with open(indexFile, 'w') as f:
                f.write(str(i))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--outdir', action='store', dest="outdir", required=True)
    parser.add_argument('--config', action="store", dest="config", required=True)

    pathes = parser.parse_args()

    if not os.path.exists(pathes.outdir) or os.path.isfile(pathes.outdir):
        print("Outdir folder doesn't exists")
        sys.exit(1)
    elif not os.path.exists(pathes.config) or not os.path.isfile(pathes.config):
        print("Config file doesn't exists")
        sys.exit(1)

    processes = []
    # 10 at a time seems to work fine
    with ThreadPoolExecutor(max_workers=10) as executor:
        for lang, url in forums.items():
            processes.append(executor.submit(get_attachments_from_url, lang, url, pathes))

    for task in as_completed(processes):
        result = task.result()
        if result:
            print(result)
