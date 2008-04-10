/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PDFComparator.java,v $
 * $Revision: 1.5 $
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

