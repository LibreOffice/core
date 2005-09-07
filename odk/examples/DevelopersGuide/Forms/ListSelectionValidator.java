/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ListSelectionValidator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:08:58 $
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

/**
 *
 * @author  fs@openoffice.org
 */
public class ListSelectionValidator extends ControlValidator
{
    /** Creates a new instance of ListSelectionValidator */
    public ListSelectionValidator()
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            short[] selectionIndexes = (short[])Value;
            if ( selectionIndexes.length > 2 )
                return "please 2 entries, at most";
        }
        catch( java.lang.Exception e )
        {
            return "oops. What's this?";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            short[] selectionIndexes = (short[])Value;
            if ( selectionIndexes.length > 2 )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }

}
