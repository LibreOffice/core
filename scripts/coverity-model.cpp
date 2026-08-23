/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* If this is modified it must be manually uploaded to coverity scan
 * at "Modeling File" in the Analysis Settings tab of
 * https://scan.coverity.com/projects/collaboraonline
 */

namespace std {
    class string {};
}

namespace CppUnit {
    class Message;
    class SourceLine;

    struct Asserter {
        static void failIf(bool shouldFail, const Message&, const SourceLine&) {
            if (shouldFail) __coverity_panic__();
        }

        static void failIf(bool shouldFail, std::string, const SourceLine&) {
            if (shouldFail) __coverity_panic__();
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
