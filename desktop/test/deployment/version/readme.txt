#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

There are three extensions in various versions:

1  version_XXX/plain.oxt has no dependencies and no license.
2a version_XXX/dependency.oxt has an unsatisfied dependency and no license.
2b version_nodependencies_XXX/dependency.oxt is identical to 2a but without the
   dependency.
3  version_XXX/license.oxt has no dependencies and a license.

The different versions are:

A  version_none contains no version element (treated as version "0").
B  version_badelement contains a bad <version val="1"/> (not allowed by the
   specification, but treated by the current implementation as version "0").
C  version_badvalue contains a bad <version value="1.a"/> (not allowed by the
   specification, but treated by the current implementation as version "1").
D  version_0.0 contains <version value="0.0"/> (same as version "0").
E  version_1.2.3 contains <version value="1.2.3"/>.
F  version_1.2.4.7 contains <version value="1.2.4.7"/>.
G  version_1.02.4.7.0 contains <version value="1.02.4.7.0"/> (same as version
   "1.2.4.7").
H  version_1.2.15.3 contains <version value="1.2.15.3"/>.

The total order among the various versions is thus

  A = B = D < C < E < F = G < H.

Things to watch for:

- If version y of extension e is to be installed and version x < y of
  extension e is already installed, then
   unopkg add e
  will replace x with y.

- If version y of extension e is to be installed and version x >= y of
  extension e is already installed, then
   unopkg add e
  will fail with an error message.

- If version y of extension e is to be installed and any version x of
  extension e is already installed, then
   unopkg add -f e
  will replace x with y.

- If version y of extension e is to be installed and any version x of
  extension e is already installed, then
   unopkg gui "Add..."
  and
   soffice "Tools - Package Manager... - Add..."
  will query with a dialog whether to replace x with y.  The dialog will have
  "OK" (replace) preselected if x < y, and "Cancel" otherwise.

- If replacing an installed version x of an extension e with a version y fails
  because y has unsatisfied dependencies, or because y has a license to which the
  user does not agree, version x is left installed afterwards.

- Checking for already installed versions of an extension is only done within a
  single layer (unopkg versus unopkg --shared; "My Packages" versus
  "OpenOffice Packages" in unopkg gui/soffice), not across layers.
