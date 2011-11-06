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




#ifndef _FILEOPEN_DIALOG_HXX_
#define _FILEOPEN_DIALOG_HXX_

#include <vector>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class FileOpenDialog
{
    // a list of filters that are provided within the SaveDialog
    struct FilterEntry
    {
        rtl::OUString maName;
        rtl::OUString maType;
        rtl::OUString maUIName;
        rtl::OUString maFilter;
        sal_Int32 maFlags;
        com::sun::star::uno::Sequence< rtl::OUString > maExtensions;

        FilterEntry() :
            maFlags( 0 ) {}
    };
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxMSF;
    com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > mxFilePicker;

    std::vector< FilterEntry > aFilterEntryList;

public :
        FileOpenDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF );
        ~FileOpenDialog();

        sal_Int16 execute();

        void setDefaultName( const rtl::OUString& );
//      void setDefaultDirectory( const rtl::OUString& );

        ::rtl::OUString getURL() const;
        ::rtl::OUString getFilterName() const;
};

#endif  // _FILEOPEN_DIALOG_HXX_

