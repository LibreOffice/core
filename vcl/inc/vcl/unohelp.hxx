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



#ifndef _VCL_UNOHELP_HXX
#define _VCL_UNOHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

namespace com {
namespace sun {
namespace star {
namespace i18n {
    class XBreakIterator;
    class XCharacterClassification;
    class XCollator;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    struct AccessibleEventObject;
}
}}}

namespace vcl
{
namespace unohelper
{

::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > VCL_DLLPUBLIC GetMultiServiceFactory();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > VCL_DLLPUBLIC CreateBreakIterator();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCharacterClassification> VCL_DLLPUBLIC CreateCharacterClassification();
::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCollator > VCL_DLLPUBLIC CreateCollator();
::rtl::OUString VCL_DLLPUBLIC CreateLibraryName( const sal_Char* pModName, sal_Bool bSUPD );
void VCL_DLLPUBLIC NotifyAccessibleStateEventGlobally( const ::com::sun::star::accessibility::AccessibleEventObject& rEventObject );
}}  // namespace vcl::unohelper

#endif  // _VCL_UNOHELP_HXX

