/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XFieldSelectionListener.java,v $
 *
 * $Revision: 1.3.192.1 $
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
/*
 * FieldSelectionActions.java
 *
 * Created on July 14, 2003, 5:42 PM
 */
package com.sun.star.wizards.ui;

/**
 *
 * @author  bc93774
 */
public interface XFieldSelectionListener
{

    public void shiftFromLeftToRight(String[] SelItems, String[] NewItems);

    public void shiftFromRightToLeft(String[] OldSelItems, String[] NewItems);

    public void moveItemUp(String Selitem);

    public void moveItemDown(String Selitem);

    public void setID(String sIncSuffix);

    public int getID();
}
