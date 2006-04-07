/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGExporter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 13:46:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.ConfigSet;

public class CGExporter extends ConfigSetItem {
    public String          cp_Name;
    public String          cp_ExporterClass;
    public boolean         cp_OwnDirectory;
    public boolean         cp_SupportsFilename;
    public String          cp_DefaultFilename;
    public String          cp_Extension;
    public String          cp_SupportedMimeTypes;
    public String          cp_Icon;
    public String          cp_TargetType;
    public boolean         cp_Binary;
    public int             cp_PageType;

    public String          targetTypeName = "";

    public ConfigSet       cp_Arguments = new ConfigSet(CGArgument.class);


    public String toString() {
        return cp_Name;
    }

    public boolean supports(String mime) {
        return (cp_SupportedMimeTypes.equals("") || cp_SupportedMimeTypes.indexOf(mime)>-1);
    }

}
