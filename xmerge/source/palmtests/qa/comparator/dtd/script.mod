<!--

   OpenOffice.org - a multi-platform office productivity suite

   $RCSfile: script.mod,v $

   $Revision: 1.3 $

   last change: $Author: ihi $ $Date: 2006-08-01 12:51:40 $

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
