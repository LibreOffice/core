#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import requests
from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry
import magic
import tempfile
import os
import shutil
from attachment_mimetypes import mimetypes
from concurrent.futures import ThreadPoolExecutor, as_completed

# https://wiki.documentfoundation.org/Website/Web_Sites_services#Unofficial_and_Related_Pages
languages = {
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
}

def get_attachments_from_url(lang, url):

    print("Checking " + url)

    startPoint = 0

    # Keep the index and resume from there
    indexFile = lang + ".index"
    if os.path.isfile(indexFile):
        with open(indexFile) as f:
            startPoint = int(f.readline().rstrip()) + 1
    else:
        if lang == 'hu':
            startPoint = 1300

    session = requests.Session()
    retry = Retry(connect=3, backoff_factor=0.5)
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)

    invalidCount = 0
    for i in range(startPoint, 999999):
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

            if content_type == 'application/octet-stream':
                r = session.get(fileUrl, allow_redirects=True)
                with tempfile.NamedTemporaryFile() as tmp:
                    tmp.write(r.content)
                    mimetype = magic.from_file(tmp.name, mime=True)
                    if mimetype in mimetypes:
                        suffix = mimetypes[mimetype]
                        try:
                            os.mkdir(suffix)
                        except:
                            pass

                        download = suffix + '/' + "forum-" + lang + '-' + str(i) + '.' + suffix

                        print("Downloading as " + download)
                        shutil.copy(tmp.name, download)

            # Save the index
            with open(indexFile, 'w') as f:
                f.write(str(i))

if __name__ == '__main__':

    processes = []
    # 10 at a time seems to work fine
    with ThreadPoolExecutor(max_workers=10) as executor:
        for lang, url in languages.items():
            processes.append(executor.submit(get_attachments_from_url, lang, url))

    for task in as_completed(processes):
        result = task.result()
        if result:
            print(result)
