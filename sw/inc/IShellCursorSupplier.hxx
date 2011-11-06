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



#ifndef ISHELLCURSORSUPPLIER_HXX
#define ISHELLCURSORSUPPLIER_HXX


class SwPaM;


namespace sw {

/** The Undo actions need to create new Shell cursors.
    Just creating new SwPaMs in the Undo actions is not sufficient,
    because only Shell cursors are corrected by doccorr.cxx.
 */
class IShellCursorSupplier
{
public:
    virtual ~IShellCursorSupplier() { }
    virtual SwPaM & CreateNewShellCursor() = 0;
    virtual SwPaM & GetCurrentShellCursor() = 0;
};

} // namespace sw

#endif // ISHELLCURSORSUPPLIER_HXX
