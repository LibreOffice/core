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

