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



package complex.sfx2.undo;

import com.sun.star.lang.XMultiServiceFactory;
import org.openoffice.test.tools.DocumentType;

/**
 * @author frank.schoenheit@oracle.com
 */
public class DrawDocumentTest extends DrawingOrPresentationDocumentTest
{
    public DrawDocumentTest( XMultiServiceFactory i_orb ) throws com.sun.star.uno.Exception
    {
        super( i_orb, DocumentType.DRAWING );
    }

    public String getDocumentDescription()
    {
        return "drawing document";
    }
}
