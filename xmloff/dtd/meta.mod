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


<!ELEMENT meta:generator (%cString;)>

<!ELEMENT dc:title (%cString;)>

<!ELEMENT dc:description (%cString;)>

<!ELEMENT dc:subject (%cString;)>

<!ELEMENT meta:keywords (meta:keyword)*>
<!ELEMENT meta:keyword (%cString;)>

<!ELEMENT meta:initial-creator (%cString;)>

<!ELEMENT dc:creator (%cString;)>

<!ELEMENT meta:printed-by (%cString;)>

<!ELEMENT meta:creation-date (%cTimeInstance;)>

<!ELEMENT dc:date (%cTimeInstance;)>

<!ELEMENT meta:print-date (%cTimeInstance;)>

<!ELEMENT meta:template EMPTY>
<!ATTLIST meta:template xlink:type (simple) #FIXED "simple">
<!ATTLIST meta:template xlink:actuate (onRequest) "onRequest">
<!ATTLIST meta:template xlink:href %uriReference; #REQUIRED>
<!ATTLIST meta:template xlink:title %string; #IMPLIED>
<!ATTLIST meta:template meta:date %timeInstance; #IMPLIED>

<!ELEMENT meta:auto-reload EMPTY>
<!ATTLIST meta:auto-reload xlink:type (simple) #IMPLIED>
<!ATTLIST meta:auto-reload xlink:show (replace) #IMPLIED>
<!ATTLIST meta:auto-reload xlink:actuate (onLoad) #IMPLIED>
<!ATTLIST meta:auto-reload xlink:href %uriReference; #IMPLIED>
<!ATTLIST meta:auto-reload meta:delay %timeDuration; "P0S">

<!ELEMENT meta:hyperlink-behaviour EMPTY>
<!ATTLIST meta:hyperlink-behaviour office:target-frame-name %targetFrameName; #IMPLIED>
<!ATTLIST meta:hyperlink-behaviour xlink:show (new|replace) #IMPLIED>

<!ELEMENT dc:language (%cLanguage;)>

<!ELEMENT meta:editing-cycles (%cPositiveInteger;)>

<!ELEMENT meta:editing-duration (%cTimeDuration;)>

<!ELEMENT meta:user-defined (%cString;)>
<!ATTLIST meta:user-defined meta:name %string; #REQUIRED>

<!ELEMENT meta:document-statistic EMPTY>
<!ATTLIST meta:document-statistic meta:page-count %positiveInteger; #IMPLIED
	meta:table-count %nonNegativeInteger; #IMPLIED
	meta:draw-count %nonNegativeInteger; #IMPLIED
	meta:image-count %nonNegativeInteger; #IMPLIED
	meta:ole-object-count %nonNegativeInteger; #IMPLIED
	meta:paragraph-count %nonNegativeInteger; #IMPLIED
	meta:word-count %nonNegativeInteger; #IMPLIED
	meta:character-count %nonNegativeInteger; #IMPLIED
	meta:row-count %nonNegativeInteger; #IMPLIED
	meta:cell-count %nonNegativeInteger; #IMPLIED
	meta:object-count %positiveInteger; #IMPLIED>
