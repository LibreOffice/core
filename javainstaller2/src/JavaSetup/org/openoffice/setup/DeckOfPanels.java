/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DeckOfPanels.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.setup;

import java.util.HashMap;

public class DeckOfPanels {

    private PanelController mCurrent;
    private HashMap mDeck;

    public DeckOfPanels() {
        mDeck = new HashMap();
        mCurrent = null;
    }

    void addPanel(PanelController panel, String name) {
            mDeck.put(name, panel);
    }

    public PanelController getCurrentPanel () {
        return mCurrent;
    }

    public PanelController setCurrentPanel(String name) {
        mCurrent = (PanelController)mDeck.get(name);
        return mCurrent;
    }
}
