<!--
	$Id: meta.mod,v 1.2 2000-11-23 11:01:53 sab Exp $

   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
   901 San Antonio Road, Palo Alto, CA 94303, USA

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2.1, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA  02111-1307  USA


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc..

   Copyright: 2000 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

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
	meta:table-count %positiveInteger; #IMPLIED
	meta:draw-count %positiveInteger; #IMPLIED
	meta:ole-object-count %positiveInteger; #IMPLIED
	meta:paragraph-count %positiveInteger; #IMPLIED
	meta:word-count %positiveInteger; #IMPLIED
	meta:character-count %positiveInteger; #IMPLIED
	meta:row-count %positiveInteger; #IMPLIED
	meta:cell-count %positiveInteger; #IMPLIED
	meta:object-count %positiveInteger; #IMPLIED>
