/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentType.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:08:07 $
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
import com.sun.star.uno.*;

/** a helper "enumeration class" for classifying a document type
*/
public class DocumentType extends com.sun.star.uno.Enum
{
    private DocumentType( int value )
    {
        super( value );
    }

    public static DocumentType getDefault()
    {
        return WRITER;
    }

    public static final DocumentType WRITER = new DocumentType(0);
    public static final DocumentType CALC = new DocumentType(1);
    public static final DocumentType DRAWING = new DocumentType(2);
    public static final DocumentType UNKNOWN = new DocumentType(-1);

    public static DocumentType fromInt(int value)
    {
        switch(value)
        {
            case 0: return WRITER;
            case 1: return CALC;
            case 2: return DRAWING;
            default: return UNKNOWN;
        }
    }
};

