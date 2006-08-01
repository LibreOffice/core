/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OperandLookup.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:06:55 $
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
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;
import java.util.HashMap;

import org.openoffice.xmerge.util.Debug;

/**
  * A lookup table containing information about operands
  */
public class OperandLookup extends SymbolLookup {

    /**
    * The default constructor - invokes {@link #initialize() initialize()}
    */
    public OperandLookup() {
        initialize();
    }

    /**
     * Initialize the lookup table for operands
     */
    public void initialize() {
        if ((stringToID != null) || (idToString != null)) {
            return;
        }
        stringToID = new HashMap();
        idToString = new HashMap();
        addEntry("CELL_REFERENCE", TokenConstants.TREF);
        addEntry("CELL_AREA_REFERENCE", TokenConstants.TAREA);
        addEntry("INTEGER", TokenConstants.TNUM);
        addEntry("NUMBER", TokenConstants.TNUM);
        addEntry("STRING", TokenConstants.TSTRING);
        addEntry("NAME", TokenConstants.TNAME);
        addEntry("3D_CELL_REFERENCE", TokenConstants.TREF3D);
        addEntry("3D_CELL_AREA_REFERENCE", TokenConstants.TAREA3D);
    }

}
