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



#ifndef _BASCTL_DLGEDOBJ_HXX
#define _BASCTL_DLGEDOBJ_HXX

#include <svx/svdouno.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

#include <vector>
#include <map>

#include <boost/optional.hpp>

typedef ::std::multimap< sal_Int16, ::rtl::OUString, ::std::less< sal_Int16 > > IndexToNameMap;


class DlgEdForm;
class DlgEditor;

//============================================================================
// DlgEdObj
//============================================================================

class DlgEdObj: public SdrUnoObj
{
private:
    friend class DlgEditor;
    friend class DlgEdFactory;
    friend class DlgEdPropListenerImpl;
    friend class DlgEdForm;

    sal_Bool        bIsListening;
    DlgEdForm*      pDlgEdForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener> m_xPropertyChangeListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>  m_xContainerListener;

protected:
    DlgEdObj(
        SdrModel& rSdrModel);
    DlgEdObj(
        SdrModel& rSdrModel,
        const ::rtl::OUString& rModelName,
             const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac);
    virtual ~DlgEdObj();

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    DECL_LINK(OnCreate, void* );

    using SfxListener::StartListening;
    void StartListening();
    using SfxListener::EndListening;
    void EndListening(sal_Bool bRemoveListener = sal_True);
    sal_Bool    isListening() const { return bIsListening; }

    virtual bool TransformSdrToControlCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformSdrToFormCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformControlToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );
    virtual bool TransformFormToSdrCoordinates(
        sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
        sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut );

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;
    virtual void clonedFrom(const DlgEdObj* _pSource);                          // not working yet

    virtual void SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    virtual DlgEdForm* GetDlgEdForm() const { return pDlgEdForm; }

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual sal_Bool        supportsService( const sal_Char* _pServiceName ) const;
    virtual ::rtl::OUString GetDefaultName() const;
    virtual ::rtl::OUString GetUniqueName() const;

    virtual sal_Int32   GetStep() const;
    virtual void        UpdateStep();

    virtual void SetDefaults();
    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > GetControl() const;

    virtual void PositionAndSizeChange( const ::com::sun::star::beans::PropertyChangeEvent& evt );
    virtual void SAL_CALL NameChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL TabIndexChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);

    // PropertyChangeListener
    virtual void SAL_CALL _propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // ContainerListener
    virtual void SAL_CALL _elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    virtual void SetLayer(SdrLayerID nLayer);
};


//============================================================================
// DlgEdForm
//============================================================================

class DlgEdForm: public DlgEdObj
{
private:
    friend class DlgEditor;
    friend class DlgEdFactory;

    DlgEditor* pDlgEditor;
    ::std::vector<DlgEdObj*> pChilds;

    mutable ::boost::optional< ::com::sun::star::awt::DeviceInfo >   mpDeviceInfo;

    void    ImplInvalidateDeviceInfo();

protected:
    DlgEdForm(SdrModel& rSdrModel);
    virtual ~DlgEdForm();

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    virtual void SetDlgEditor( DlgEditor* pEditor );
    virtual DlgEditor* GetDlgEditor() const { return pDlgEditor; }

    virtual void AddChild( DlgEdObj* pDlgEdObj );
    virtual void RemoveChild( DlgEdObj* pDlgEdObj );
    virtual ::std::vector<DlgEdObj*> GetChilds() const { return pChilds; }

    virtual void UpdateStep();

    virtual void SetRectFromProps();
    virtual void SetPropsFromRect();

    virtual void PositionAndSizeChange( const ::com::sun::star::beans::PropertyChangeEvent& evt );

    virtual void UpdateTabIndices();
    virtual void UpdateTabOrder();
    virtual void UpdateGroups();
    virtual void UpdateTabOrderAndGroups();

    ::com::sun::star::awt::DeviceInfo getDeviceInfo() const;
};

#endif // _BASCTL_DLGEDOBJ_HXX

