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



#ifndef SC_LNKTRANS_HXX
#define SC_LNKTRANS_HXX

#include <svtools/transfer.hxx>


class ScLinkTransferObj : public TransferableHelper
{
private:
    String  aLinkURL;
    String  aLinkText;

public:
            ScLinkTransferObj();
    virtual ~ScLinkTransferObj();

    void                SetLinkURL( const String& rURL, const String& rText );

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();
    virtual void        DragFinished( sal_Int8 nDropAction );
};

#endif

