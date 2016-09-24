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

The extensions in the subdirectories of the update folder are used for
testing the online update feature of extensions. The folder such as
simple,
license,
defect, etc. contain extensions which can be installed in OOo. The
corresponding update information file and the update are located on
the extensions.openoffice.org website (cvs: extensions/www/testarea). For example:

desktop/test/deployment/update/simple/plain1.oxt

is version 1 of this extension and it references

http://extensions.openoffice.org/testarea/desktop/simple/update/plain1.update.xml

which in turn references version 2 at

http://extensions.openoffice.org/testarea/desktop/simple/update/plain1.oxt


To have all in one place the update information file and the update are also contained
in the desktop project. They are in the update subfolder of the different test folders.
For example

.../update/simple/update
.../update/license/update
.../update/updatefeed/update


The different test folders for the update are also committed in project extensions/www
so that the files can be obtain via an URL. The structure and the contents is about the
same as the content
of desktop/test/deployment/update
For example in

extensions/www/testarea/desktop

are the subfolder defect, simple, updatefeed, wrong_url, etc.
they contain the extensions which are installed directly by the Extension Manager.
These folders contain also the update subfolder which contains the update information
and the actual updates.
