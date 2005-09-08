/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PDFComparator.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:41:48 $
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

import java.io.IOException;
import util.compare.DocComparator;
import lib.TestParameters;
import util.compare.GraphicalComparator;
import convwatch.PropertyName;

class PDFComparator extends GraphicalComparator implements DocComparator
{
    protected PDFComparator(TestParameters aParams)
        {
            super(aParams);
        }

    static DocComparator getInstance(TestParameters aParams)
        {
            aParams.put(convwatch.PropertyName.DOC_COMPARATOR_REFERENCE_TYPE, "pdf");
            PDFComparator a = new PDFComparator(aParams);
            return a;
        }

    // all in GraphicalComparator implemented.
    // public boolean isReferenceExistent() throws IOException;
    //
    // public void createReference() throws IOException;
    //
    // public boolean compare() throws IOException;
    //
    //
    // public boolean isDiffReferenceExistent() throws IOException;
    //
    // public void createDiffReference() throws IOException;
    //
    // public boolean compareDiff() throws IOException;
}

