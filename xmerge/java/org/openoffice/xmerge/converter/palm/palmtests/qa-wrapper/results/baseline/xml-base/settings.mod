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

<!ELEMENT office:settings (config:config-item-set+)>

<!ENTITY % items	"(config:config-item |
			config:config-item-set |
			config:config-item-map-named |
			config:config-item-map-indexed)+">

<!ELEMENT config:config-item-set %items;>
<!ATTLIST config:config-item-set config:name CDATA #REQUIRED>

<!ELEMENT config:config-item (#PCDATA)>
<!ATTLIST config:config-item config:name CDATA #REQUIRED
			config:type (boolean | short | int | long | double | string | datetime | base64Binary) #REQUIRED>

<!ELEMENT config:config-item-map-named (config:config-item-map-entry)+>
<!ATTLIST config:config-item-map-named config:name CDATA #REQUIRED>

<!ELEMENT config:config-item-map-indexed (config:config-item-map-entry)+>
<!ATTLIST config:config-item-map-indexed config:name CDATA #REQUIRED>

<!ELEMENT config:config-item-map-entry %items;>
<!ATTLIST config:config-item-map-entry config:name CDATA #IMPLIED>
