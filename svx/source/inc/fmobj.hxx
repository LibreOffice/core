/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmobj.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:32:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_FMOBJ_HXX
#define _SVX_FMOBJ_HXX

#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

class FmFormView;

//==================================================================
// FmFormObj
//==================================================================
class FmXForms;
class FmFormObj: public SdrUnoObj
{
    friend class FmForm;
    friend class FmFormPage;
    friend class FmFormPageImpl;
    friend class FmFormObjFactory;
    friend class FmXUndoEnvironment;
    friend class SvxFmDrawPage;
    friend class SvxFmMSFactory;

    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >  aEvts;  // events des Objects
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>   m_aEventsHistory;
                // valid if and only if m_pEnvironmentHistory != NULL, this are the events which we're set when
                // m_pEnvironmentHistory was created

    FmFormView*     m_pControlCreationView;
    ULONG       m_nControlCreationEvent;

    // Informationen fuer die Controlumgebung
    // werden nur vorgehalten, wenn ein Object sich nicht in einer Objectliste befindet
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>     m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    m_xEnvironmentHistory;
    sal_Int32           m_nPos;
    sal_Int32           m_nType;

    OutputDevice*       m_pLastKnownRefDevice;
                            // the last ref device we know, as set at the model
                            // only to be used for comparison with the current ref device!

public:
    TYPEINFO();

protected:
    FmFormObj(const ::rtl::OUString& rModelName,sal_Int32 _nType);
    FmFormObj(sal_Int32 _nType);

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& GetParent() const {return m_xParent;}
    void  SetObjEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xForm, sal_Int32 nIdx = -1,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& rEvts= ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >());
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& GetEvents() const {return aEvts;}
    sal_Int32 GetPos() const { return m_nPos; }

public:
    virtual ~FmFormObj();
    virtual void SetPage(SdrPage* pNewPage);

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void ReformatText();

    virtual SdrObject*  Clone() const;
    // #116235# virtual SdrObject*  Clone(SdrPage* pPage, SdrModel* pModel) const;
    virtual void        operator= (const SdrObject& rObj);

    virtual void clonedFrom(const FmFormObj* _pSource);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> ensureModelEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rSourceContainer, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> _rTopLevelDestContainer);

    /** returns the type of this form object. See fmglob.hxx
    */
    sal_Int32   getType() const;

protected:
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    DECL_LINK(OnCreate, void* );

    // #i70852# overload Layer interface to force to FormColtrol layer
    virtual SdrLayerID GetLayer() const;
    virtual void NbcSetLayer(SdrLayerID nLayer);
};


#endif // _FM_FMOBJ_HXX

