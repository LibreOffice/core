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



#include "filebuff.hxx"

#include <string.h>
#include <fstream>
#include <ctype.h>


bool
LoadXmlFile( Buffer &           o_rBuffer,
             const char *       i_sXmlFilePath )
{
    std::ifstream aXmlFile;

    aXmlFile.open(i_sXmlFilePath, std::ios::in
#if defined(WNT) || defined(OS2)
                                          | std::ios::binary
#endif // WNT
    );

    if (! aXmlFile)
        return false;

    // Prepare buffer:
    aXmlFile.seekg(0, std::ios::end);
    unsigned long nBufferSize = (unsigned long) aXmlFile.tellg();
    o_rBuffer.SetSize(nBufferSize + 1);
    o_rBuffer.Data()[nBufferSize] = '\0';
    aXmlFile.seekg(0);

    // Read file:
    aXmlFile.read(o_rBuffer.Data(), (int) nBufferSize);
    const bool ret = !aXmlFile.fail();
    aXmlFile.close();
    return ret;
}

