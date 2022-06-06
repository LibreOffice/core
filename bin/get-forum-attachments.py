#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import requests
import magic
import tempfile
import os
import shutil
from attachment_mimetypes import mimetypes
from concurrent.futures import ThreadPoolExecutor, as_completed

# https://forum.openoffice.org/
languages = ['en', 'es', 'fr', 'hu', 'it', 'ja', 'nl', 'pl', 'vi', 'de']

def get_attachments_from_url(lang):

    if lang == 'de':
        url = "https://www.openoffice-forum.de/download/file.php?id="
        print("Checking https://www.openoffice-forum.de")
    else:
        domain = "https://forum.openoffice.org/"
        url = domain + lang + "/forum/download/file.php?id="
        print("Checking " + domain + lang)

    startPoint = 0
    if lang == 'hu':
        startPoint = 1300

    invalidCount = 0
    for i in range(startPoint, 999999):
        fileUrl = url + str(i)
        h = requests.head(fileUrl, allow_redirects=True)
        header = h.headers
        content_type = header.get('content-type')
        if "html" in content_type:
            # Let's assume this is an invalid file link
            invalidCount += 1

            # Let's assume, if we get 100 invalid file, that there a no more files
            if invalidCount == 100:
                print(str(i))
                print("No more attachments found in " + url)
                break
        else:
            invalidCount = 0

            if content_type == 'application/octet-stream':
                r = requests.get(fileUrl, allow_redirects=True)
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

                        if os.path.isfile(download):
                            print("Assuming " + download + " is up to date")
                            continue
                        print("Downloading as " + download)
                        shutil.copy(tmp.name, download)

if __name__ == '__main__':

    # 4 at a time seems to work fine
    with ThreadPoolExecutor(max_workers=4) as executor:
        for lang in languages:
            executor.submit(get_attachments_from_url, lang)

