/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_COLUMNPEER_HXX
#define DBAUI_COLUMNPEER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaui
{
    class OFieldDescription;
    class OColumnPeer   :   public VCLXWindow
    {
        OFieldDescription*                                                              m_pActFieldDescr;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xColumn;
    public:

        OColumnPeer(Window* _pParent
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);

        void setColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn);
        void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xCon);
        void setEditWidth(sal_Int32 _nWidth);
        // VCLXWindow
        virtual void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
    };
}   // namespace dbaui
#endif //DBAUI_COLUMNPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
