/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 *************************************************************************/

package complex.toolkit.awtgrid;

import com.sun.star.awt.grid.XGridColumn;
import com.sun.star.awt.grid.XGridColumnListener;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XEventListener;
import com.sun.star.style.HorizontalAlignment;
import com.sun.star.util.XCloneable;

/**
 * a dummy implementation of css.awt.grid.XGridColumn
 * @author frank.schoenheit@oracle.com
 */
public class DummyColumn implements XGridColumn
{
    public DummyColumn()
    {
    }

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
