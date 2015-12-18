# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import requests, sys

def main():
    if len(sys.argv) != 3:
        print("Invalid number of parameters")
        sys.exit(1)

    url = "http://localhost:8000/upload/"
    files = {'symbols': open(sys.argv[1], 'rb'), 'comment':sys.argv[2]}
    comment = {'comment': sys.argv[2]}
    r = requests.post(url, files = files, data = {"comment":"whatever", "tempt":"tempt"})
    print(r)

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
