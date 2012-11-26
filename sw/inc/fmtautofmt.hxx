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


#ifndef _FMTAUTOFMT_HXX
#define _FMTAUTOFMT_HXX


#include <svl/poolitem.hxx>
#include <format.hxx>
#include <boost/shared_ptr.hpp>

// ATT_AUTOFMT *********************************************


class SwFmtAutoFmt: public SfxPoolItem
{
    boost::shared_ptr<SfxItemSet> mpHandle;

public:
    SwFmtAutoFmt( sal_uInt16 nWhich = RES_TXTATR_AUTOFMT );

    // single argument ctors shall be explicit.
    virtual ~SwFmtAutoFmt();

    // @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt( const SwFmtAutoFmt& rAttr );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt & operator= (const SwFmtAutoFmt &);
public:

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void SetStyleHandle( boost::shared_ptr<SfxItemSet> pHandle ) { mpHandle = pHandle; }
    const boost::shared_ptr<SfxItemSet> GetStyleHandle() const { return mpHandle; }
          boost::shared_ptr<SfxItemSet> GetStyleHandle() { return mpHandle; }
};

#endif

