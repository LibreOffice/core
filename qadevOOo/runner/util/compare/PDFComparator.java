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

package util.compare;

import util.compare.DocComparator;
import lib.TestParameters;
import util.compare.GraphicalComparator;

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

    // public void createReference() throws IOException;

    // public boolean compare() throws IOException;


    // public boolean isDiffReferenceExistent() throws IOException;

    // public void createDiffReference() throws IOException;

    // public boolean compareDiff() throws IOException;
}

