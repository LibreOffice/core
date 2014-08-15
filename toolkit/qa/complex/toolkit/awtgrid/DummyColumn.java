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

package complex.toolkit.awtgrid;

import com.sun.star.awt.grid.XGridColumn;
import com.sun.star.awt.grid.XGridColumnListener;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XEventListener;
import com.sun.star.style.HorizontalAlignment;
import com.sun.star.util.XCloneable;

/**
 * a dummy implementation of css.awt.grid.XGridColumn
 */
public class DummyColumn implements XGridColumn
{
    public Object getIdentifier()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setIdentifier( Object o )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getColumnWidth()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setColumnWidth( int i )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getMinWidth()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setMinWidth( int i )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getMaxWidth()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setMaxWidth( int i )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public boolean getResizeable()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setResizeable( boolean bln )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getFlexibility()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setFlexibility( int i ) throws IllegalArgumentException
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public HorizontalAlignment getHorizontalAlign()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setHorizontalAlign( HorizontalAlignment ha )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public String getTitle()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setTitle( String string )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public String getHelpText()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setHelpText( String string )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getIndex()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public int getDataColumnIndex()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void setDataColumnIndex( int i )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void addGridColumnListener( XGridColumnListener xl )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void removeGridColumnListener( XGridColumnListener xl )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void dispose()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void addEventListener( XEventListener xl )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public void removeEventListener( XEventListener xl )
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }

    public XCloneable createClone()
    {
        throw new UnsupportedOperationException( "Not supported yet." );
    }
}
