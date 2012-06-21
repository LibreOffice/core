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
     * @return the primitive <code>int</code> value of this object.
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
