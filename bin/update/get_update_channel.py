#!/usr/bin/python3
# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
from config import parse_config


def main():
    if len(sys.argv) < 2:
        sys.exit(1)

    update_config = sys.argv[1]
    config = parse_config(update_config)
    print(config.channel)


if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
