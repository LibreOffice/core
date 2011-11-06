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


#ifndef DBAUI_COLUMNCONTROLWINDOW_HXX
#define DBAUI_COLUMNCONTROLWINDOW_HXX

#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    // =============================================================================================
    // OColumnControlWindow
    // =============================================================================================
    class OColumnControlWindow : public OFieldDescControl
    {
        ::com::sun::star::lang::Locale      m_aLocale;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        OTypeInfoMap                m_aDestTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aDestTypeInfoIndex;

        mutable TOTypeInfoSP        m_pTypeInfo; // default type
        String                      m_sTypeNames;       // these type names are the ones out of the resource file
        ::rtl::OUString             m_sAutoIncrementValue;
        sal_Bool                    m_bAutoIncrementEnabled;
    protected:
        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );

        virtual ::com::sun::star::lang::Locale  GetLocale() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos);
        virtual sal_Bool            isAutoIncrementValueEnabled() const;
        virtual ::rtl::OUString     getAutoIncrementValue() const;
        virtual void                CellModified(long nRow, sal_uInt16 nColId );

    public:
        OColumnControlWindow(Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
        virtual ~OColumnControlWindow();

        void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xCon);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection();
        virtual const OTypeInfoMap* getTypeInfo() const;
        TOTypeInfoSP getDefaultTyp() const;
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................
#endif // DBAUI_COLUMNCONTROLWINDOW_HXX
