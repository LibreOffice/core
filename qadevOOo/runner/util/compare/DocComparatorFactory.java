/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocComparatorFactory.java,v $
 * $Revision: 1.4 $
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
