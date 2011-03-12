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
.../update/updatefeed/udpate


The different test folders for the update are also commited in project extensions/www 
so that the files can be obtain via an URL. The structure and the contents is about the 
same as the content 
of desktop/test/deployment/udpate 
For example in

extensions/www/testarea/desktop

are the subfolder defect, simple, updatefeed, wrong_url, etc.
they contain the extensions which are installed directly by the Extension Manager.
These folders contain also the update subfolder which contains the update information
and the actual updates.
