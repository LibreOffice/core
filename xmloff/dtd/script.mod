<!--

    OpenOffice.org - a multi-platform office productivity suite
 
    $RCSfile: script.mod,v $
 
    $Revision: 1.10 $
 
    last change: $Author: rt $ $Date: 2005-09-09 12:32:35 $
 
    The Contents of this file are made available subject to
    the terms of GNU Lesser General Public License Version 2.1.
 
 
      GNU Lesser General Public License Version 2.1
      =============================================
      Copyright 2005 by Sun Microsystems, Inc.
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
 
-->

<!ELEMENT script:libraries (script:library-embedded | script:library-linked)*>
<!ATTLIST script:libraries xmlns:script CDATA #FIXED "http://openoffice.org/2000/script">
<!ATTLIST script:libraries xmlns:xlink CDATA #FIXED "http://www.w3.org/1999/xlink">

<!ENTITY % boolean "(true|false)">

<!ELEMENT script:library-embedded (script:module*)>
<!ATTLIST script:library-embedded script:name %string; #REQUIRED>
<!ATTLIST script:library-embedded script:readonly %boolean; #IMPLIED>

<!ELEMENT script:library-linked EMPTY>
<!ATTLIST script:library-linked script:name %string; #REQUIRED>
<!ATTLIST script:library-linked xlink:href %string; #REQUIRED>
<!ATTLIST script:library-linked xlink:type (simple) #FIXED "simple">
<!ATTLIST script:library-linked script:readonly %boolean; #IMPLIED>

<!ELEMENT script:module (script:source-code)>
<!ATTLIST script:module script:name %string; #REQUIRED>

<!ELEMENT script:source-code (#PCDATA)>


<!ENTITY % script-language "script:language %string; #REQUIRED">
<!ENTITY % event-name "script:event-name %string; #REQUIRED">
<!ENTITY % location "script:location (document|application) #REQUIRED">
<!ENTITY % macro-name "script:macro-name %string; #REQUIRED">

<!ELEMENT script:event (#PCDATA)>
<!ATTLIST script:event %script-language;
                       %event-name;
                       %location;
					   %macro-name;>
