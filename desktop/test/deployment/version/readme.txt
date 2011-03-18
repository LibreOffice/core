/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
  because y has unsatisfied dependencies, or because y has a license to wich the
  user does not agree, version x is left installed afterwards.

- Checking for already installed versions of an extension is only done within a
  single layer (unopkg versus unopkg --shared; "My Packages" versus
  "OpenOffice Packages" in unopkg gui/soffice), not across layers.
