<!--

   OpenOffice.org - a multi-platform office productivity suite

   $RCSfile: settings.mod,v $

   $Revision: 1.3 $

   last change: $Author: ihi $ $Date: 2006-08-01 12:56:08 $

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
