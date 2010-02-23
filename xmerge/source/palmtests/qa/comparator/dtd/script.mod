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

<!ELEMENT script:library-embedded (script:module*)>
<!ATTLIST script:library-embedded script:name %string; #REQUIRED>
<!ATTLIST script:library-embedded script:password %string; #IMPLIED>

<!ELEMENT script:library-linked EMPTY>
<!ATTLIST script:library-linked script:name %string; #REQUIRED>
<!ATTLIST script:library-linked xlink:href %string; #REQUIRED>
<!ATTLIST script:library-linked xlink:type (simple) #FIXED "simple">

<!ELEMENT script:module (#PCDATA)>
<!ATTLIST script:module script:name %string; #REQUIRED>
<!ATTLIST script:module script:language %string; #IMPLIED>


<!ENTITY % script-language "script:language %string; #REQUIRED">
<!ENTITY % event-name "script:event-name %string; #REQUIRED">
<!ENTITY % location "script:location (document|application) #REQUIRED">
<!ENTITY % macro-name "script:macro-name %string; #REQUIRED">

<!ELEMENT script:event (#PCDATA)>
<!ATTLIST script:event %script-language;
                       %event-name;
                       %location;
					   %macro-name;>
