/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
