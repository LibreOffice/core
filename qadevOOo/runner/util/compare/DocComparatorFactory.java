/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocComparatorFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:41:17 $
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

package util.compare;

import lib.TestParameters;
import util.compare.GraphicalComparator;

public class DocComparatorFactory
{
    /**
     * @param  s is a name like 'ooo' 'pdf' or 'msoffice'
     * @return a new DocComparator Object
     */
    static public DocComparator createComparator(String s, TestParameters aParams) throws IllegalArgumentException
        {
            if (s.toLowerCase().equals("gfx") || s.toLowerCase().equals("graphical"))
            {
                return GraphicalComparator.getInstance(aParams);
            }
/*
            else if (s.toLowerCase().equals("xml"))
            {
                return new XMLCompare.create(aParams);
            }
*/
            else if (s.toLowerCase().equals("pdf"))
            {
                // return new PDFComparator.create(aParams);
                throw new IllegalArgumentException("PDF not implemented yet.");
            }
            else
            {
                throw new IllegalArgumentException("DocComparator for '" + s + "' not supported!");
            }

            // unreachable: return null;
        }

}
