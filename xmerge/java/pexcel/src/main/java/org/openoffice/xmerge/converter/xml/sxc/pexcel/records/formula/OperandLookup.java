/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
