<!--

  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  This file is part of OpenOffice.org.
 
  OpenOffice.org is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 3
  only, as published by the Free Software Foundation.
 
  OpenOffice.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License version 3 for more details
  (a copy is included in the LICENSE file that accompanied this code).
 
  You should have received a copy of the GNU Lesser General Public License
  version 3 along with OpenOffice.org.  If not, see
  <http://www.openoffice.org/license.html>
  for a copy of the LGPLv3 License.

-->

<!-- This module should contain entities intended for content definitions
     in several other modules. Putting all of them here should remove
     (some) order dependencies of the other module files
-->


<!-- text marks for tracking changes; usually used inside of paragraphs -->
<!ENTITY % change-marks "text:change | text:change-start | text:change-end">

<!-- (optional) text declarations; used before the first paragraph -->
<!ENTITY % text-decls "text:variable-decls?, text:sequence-decls?,
					   text:user-field-decls?, text:dde-connection-decls?, 
					   text:alphabetical-index-auto-mark-file?" >

<!-- define the types of text which may occur inside of sections -->
<!ENTITY % sectionText "(text:h|text:p|text:ordered-list|
						text:unordered-list|table:table|text:section|
						text:table-of-content|text:illustration-index|
						text:table-index|text:object-index|text:user-index|
						text:alphabetical-index|text:bibliography|
						text:index-title|%change-marks;)*">

<!ENTITY % headerText "(%text-decls;, (text:h|text:p|text:ordered-list|
						text:unordered-list|table:table|text:section|
						text:table-of-content|text:illustration-index|
						text:table-index|text:object-index|text:user-index|
						text:alphabetical-index|text:bibliography|
						text:index-title|%change-marks;)* )">

