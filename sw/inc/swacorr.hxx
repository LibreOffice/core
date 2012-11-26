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


#ifndef _SWACORR_HXX
#define _SWACORR_HXX

#include <editeng/svxacorr.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>

class SwAutoCorrect : public SvxAutoCorrect
{
    using  SvxAutoCorrect::PutText;

protected:
    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
    //      rShort ist der Stream-Name - gecryptet!
    virtual sal_Bool GetLongText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& , const String& rFileName, const String& rShort, String& rLong );

    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    //      rShort ist der Stream-Name - gecryptet!
    virtual sal_Bool PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String& rFileName, const String& rShort, SfxObjectShell& ,
                            String& );

public:
    SwAutoCorrect( const SvxAutoCorrect& rACorr );
    virtual ~SwAutoCorrect();
};


#endif

