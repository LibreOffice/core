/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TriState.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-05-17 13:30:19 $
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
