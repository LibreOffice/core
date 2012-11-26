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


#ifndef _SVX_FMOBJ_HXX
#define _SVX_FMOBJ_HXX

#include <svx/svdouno.hxx>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

class FmFormView;

//==================================================================
// FmFormObj
//==================================================================
class FmXForms;
class FmFormObj: public SdrUnoObj
{
private:
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >  aEvts;  // events des Objects
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>   m_aEventsHistory;
                // valid if and only if m_pEnvironmentHistory != NULL, this are the events which we're set when
                // m_pEnvironmentHistory was created

    FmFormView*     m_pControlCreationView;
    sal_uLong       m_nControlCreationEvent;

    // Informationen fuer die Controlumgebung
    // werden nur vorgehalten, wenn ein Object sich nicht in einer Objectliste befindet
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>     m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    m_xEnvironmentHistory;
    sal_Int32           m_nPos;
    sal_Int32           m_nType;

    OutputDevice*       m_pLastKnownRefDevice;
                            // the last ref device we know, as set at the model
                            // only to be used for comparison with the current ref device!

protected:
    virtual ~FmFormObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;
    virtual void clonedFrom(const FmFormObj* _pSource);

    SVX_DLLPUBLIC FmFormObj(SdrModel& rSdrModel, const ::rtl::OUString& rModelName, sal_Int32 _nType);
//  SVX_DLLPUBLIC FmFormObj(sal_Int32 _nType);

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>&
        GetOriginalParent() const { return m_xParent; }
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >&
        GetOriginalEvents() const { return aEvts; }
    sal_Int32
        GetOriginalIndex() const { return m_nPos; }

    void SetObjEnv(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xForm,
            const sal_Int32 nIdx,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& rEvts );
    void ClearObjEnv();

    // react on page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void ReformatText();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> ensureModelEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rSourceContainer, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> _rTopLevelDestContainer);

    /** returns the FmFormObj behind the given SdrObject

        In case the SdrObject *is* an FmFormObject, this is a simple cast. In case the SdrObject
        is a virtual object whose referenced object is an FmFormObj, then this referenced
        object is returned. In all other cases, NULL is returned.
    */
    static       FmFormObj* GetFormObject( SdrObject* _pSdrObject );
    static const FmFormObj* GetFormObject( const SdrObject* _pSdrObject );

    /** returns the type of this form object. See fmglob.hxx
    */
    sal_Int32   getType() const;

    virtual void SetUnoControlModel( const ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& _rxModel );

protected:
    virtual bool EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd );
    virtual void        BrkCreate( SdrDragStat& rStat );

    // #i70852# overload Layer interface to force to FormColtrol layer
    virtual SdrLayerID GetLayer() const;
    virtual void SetLayer(SdrLayerID nLayer);

private:
    /** isolates the control model from its form component hierarchy, i.e. removes it from
        its parent.
    */
    void    impl_isolateControlModel_nothrow();

    /** forwards the reference device of our SdrModel to the control model
    */
    void    impl_checkRefDevice_nothrow( bool _force = false );
};


#endif // _FM_FMOBJ_HXX

