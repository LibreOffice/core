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
