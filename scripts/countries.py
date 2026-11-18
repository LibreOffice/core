#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Usage:
# scripts/countries.py /path/to/node_modules/i18n-iso-countries/langs/en.json

import json
import sys

if __name__ == "__main__":
    json_path = sys.argv[1]
    with open(json_path) as stream:
        root = json.load(stream)
    countries = root["countries"]
    # eID Easy needs ~75 countries, i18n-iso-countries would have 250, limit the output to the used
    # subset.
    subset = ["AD", "AE", "AL", "AM", "AR", "AT", "AU", "AX", "AZ", "BA", "BE", "BG", "BR", "BY", "CA", "CH", "CL", "CN", "CY", "CZ", "DE", "DK", "EE", "ES", "FI", "FR", "GB", "GE", "GG", "GR", "HR", "HU", "ID", "IE", "IL", "IN", "IS", "IT", "JP", "KE", "KR", "KZ", "LI", "LT", "LU", "LV", "MC", "MD", "ME", "MK", "MT", "MX", "NL", "NO", "NZ", "PH", "PL", "PT", "QA", "RO", "RS", "RU", "SA", "SE", "SG", "SI", "SK", "SM", "TR", "TW", "UA", "US", "VA", "XK", "ZA"]
    for code in subset:
        value = countries[code]
        if type(value) == str:
            name = value
        elif type(value) == list:
            name = value[0]
        print("{}: _('{}'),".format(code, name))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
