/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;
import java.util.HashMap;

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
        stringToID = new HashMap<String, Integer>();
        idToString = new HashMap<Integer, String>();
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
