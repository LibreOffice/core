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



#ifndef X2C_FILEBUFF_HXX
#define X2C_FILEBUFF_HXX


class Buffer
{
  public:
                        Buffer()                : s(0) {}
                        ~Buffer()               { if (s) delete [] s; }
                        operator const char *() const
                                                { return s; }
    char *              Data()                  { return s; }
    void                SetSize(
                           unsigned long                i_size )
                                                { if (s) delete [] s; s = new char [i_size]; }
  private:
    char * s;
};


bool                LoadXmlFile(
                        Buffer &            o_rBuffer,
                        const char *        i_sXmlFilePath );



#endif
