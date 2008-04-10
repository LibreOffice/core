/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TriState.java,v $
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

package convwatch;

public class TriState
{
    public static final TriState TRUE = new TriState(1);
    public static final TriState FALSE = new TriState(0);
    public static final TriState UNSET = new TriState(-1);

    int m_nValue;

    /**
       Allocates a <code>TriState</code> object representing the
       <code>value</code> argument.

       @param   value   the value of the <code>TriState</code>.
    */
    public TriState(int value)
        {
            m_nValue = value;
        }

    /**
       Returns the value of this TriState object as an int
       @returns the primitive <code>int</code> value of this object.
    */
    public int intValue()
        {
            return m_nValue;
        }
    /**
       Returns <code>true</code> if and only if the argument is not
       <code>null</code> and is a <code>TriState</code> object that
       contains the same <code>int</code> value as this object.

       @param   obj   the object to compare with.
       @return  <code>true</code> if the objects are the same;
                <code>false</code> otherwise.
    */

    public boolean equals(Object obj)
        {
            if ((obj != null) &&
                (obj instanceof TriState))
            {
                return m_nValue == ((TriState)obj).intValue();
            }
            return false;
        }
}
