/*************************************************************************
 *
 *  $RCSfile: CustomAnimationEffect.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:42:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_CUSTOMANIMATIONEFFECT_HXX
#define _SD_CUSTOMANIMATIONEFFECT_HXX

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XTIMECONTAINER_HPP_
#include <com/sun/star/animations/XTimeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAUDIO_HPP_
#include <com/sun/star/animations/XAudio.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _UTL_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#include <list>
#include <map>


namespace sd {

// --------------------------------------------------------------------

enum EValue { VALUE_FROM, VALUE_TO, VALUE_BY, VALUE_FIRST, VALUE_LAST };

class CustomAnimationEffect;
class AnimationTrigger;

class CustomAnimationPreset;
typedef boost::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;

typedef boost::shared_ptr< CustomAnimationEffect > CustomAnimationEffectPtr;

typedef std::list< CustomAnimationEffectPtr > EffectSequence;

class EffectSequenceHelper;

class CustomAnimationEffect
{
    friend class MainSequence;
    friend class EffectSequenceHelper;

public:
    CustomAnimationEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    virtual ~CustomAnimationEffect();

    const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& getNode() const { return mxNode; }
    void setNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void replaceNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    CustomAnimationEffectPtr clone() const;

    // attributes
    const rtl::OUString&    getPresetId() const { return maPresetId; }
    const rtl::OUString&    getPresetSubType() const { return maPresetSubType; }
    const rtl::OUString&    getProperty() const { return maProperty; }
    sal_Int16               getPresetClass() const { return mnPresetClass; }

    sal_Int16       getNodeType() const { return mnNodeType; }
    void            setNodeType( sal_Int16 nNodeType );

    ::com::sun::star::uno::Any              getRepeatCount() const;
    void            setRepeatCount( const ::com::sun::star::uno::Any& rRepeatCount );

    ::com::sun::star::uno::Any              getEnd() const;
    void            setEnd( const ::com::sun::star::uno::Any& rEnd );

    sal_Int16       getFill() const;
    void            setFill( sal_Int16 nFill );

    double          getBegin() const { return mfBegin; }
    void            setBegin( double fBegin );

    double          getDuration() const { return mfDuration; }
    void            setDuration( double fDuration );

    double          getAbsoluteDuration() const { return mfAbsoluteDuration; }

    const String&   getName() const { return maName; }
    void            setName( const String& rName ) { maName = rName; }

    sal_Int16       getIterateType() const { return mnIterateType; }
    void            setIterateType( sal_Int16 nIterateType );

    double          getIterateInterval() const { return mfIterateInterval; }
    void            setIterateInterval( double fIterateInterval );

    ::com::sun::star::uno::Any  getTarget() const { return maTarget; }
    void                        setTarget( const ::com::sun::star::uno::Any& rTarget );

    sal_Bool        hasAfterEffect() const { return mbHasAfterEffect; }
    void            setHasAfterEffect( sal_Bool bHasAfterEffect ) { mbHasAfterEffect = bHasAfterEffect; }

    ::com::sun::star::uno::Any  getDimColor() const { return maDimColor; }
    void                        setDimColor( ::com::sun::star::uno::Any aDimColor ) { maDimColor = aDimColor; }

    sal_Int32       getMasterRel() const { return mnMasterRel; }
    void            setMasterRel( sal_Int32 nMasterRel ) { mnMasterRel = nMasterRel; }

    sal_Int32       getParaDepth() const { return mnParaDepth; }

    sal_Bool        hasText() const { return mbHasText; }

    sal_Int16       getCommand() const { return mnCommand; }

    double          getAcceleration() const { return mfAcceleration; }
    void            setAcceleration( double fAcceleration );

    double          getDecelerate() const { return mfDecelerate; }
    void            setDecelerate( double fDecelerate );

    sal_Bool        getAutoReverse() const { return mbAutoReverse; }
    void            setAutoReverse( sal_Bool bAutoReverse );

    ::com::sun::star::uno::Any  getProperty( sal_Int32 nNodeType, const rtl::OUString& rAttributeName, EValue eValue );
    bool                        setProperty( sal_Int32 nNodeType, const rtl::OUString& rAttributeName, EValue eValue, const ::com::sun::star::uno::Any& rValue );

    ::com::sun::star::uno::Any  getTransformationProperty( sal_Int32 nTransformType, EValue eValue );
    bool                        setTransformationProperty( sal_Int32 nTransformType, EValue eValue, const ::com::sun::star::uno::Any& rValue );

    ::com::sun::star::uno::Any  getColor( sal_Int32 nIndex );
    void                        setColor( sal_Int32 nIndex, const ::com::sun::star::uno::Any& rColor );

    ::com::sun::star::uno::Any  getRotation();
    void                        setRotation( const ::com::sun::star::uno::Any& rRotation );

    sal_Int32       getGroupId() const { return mnGroupId; }
    void            setGroupId( sal_Int32 nGroupId );

    sal_Int16       getTargetSubItem() const { return mnTargetSubItem; }
    void            setTargetSubItem( sal_Int16 nSubItem );

    bool checkForText();
    bool calculateIterateDuration();

    void setAudio( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAudio >& xAudio );
    bool getStopAudio() const;
    void setStopAudio();
    void createAudio( const ::com::sun::star::uno::Any& rSource, double fVolume = 1.0 );
    void removeAudio();
    const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAudio >& getAudio() const { return mxAudio; }

    EffectSequenceHelper*   getEffectSequence() const { return mpEffectSequence; }

    // helper
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > createAfterEffectNode() const throw (com::sun::star::uno::Exception);
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTargetShape() const;

    // static helpers
    static sal_Int32 get_node_type( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    static sal_Int32 getNumberOfSubitems( const ::com::sun::star::uno::Any& aTarget, sal_Int16 nIterateType );

protected:
    void setEffectSequence( EffectSequenceHelper* pSequence ) { mpEffectSequence = pSequence; }

private:
    sal_Int16       mnNodeType;
    rtl::OUString   maPresetId;
    rtl::OUString   maPresetSubType;
    rtl::OUString   maProperty;
    sal_Int16       mnPresetClass;
    double          mfBegin;
    double          mfDuration;                 // this is the maximum duration of the subeffects
    double          mfAbsoluteDuration;         // this is the maximum duration of the subeffects including possible iterations
    sal_Int32       mnGroupId;
    sal_Int16       mnIterateType;
    double          mfIterateInterval;
    sal_Int32       mnParaDepth;
    sal_Bool        mbHasText;
    double          mfAcceleration;
    double          mfDecelerate;
    sal_Bool        mbAutoReverse;
    sal_Int16       mnTargetSubItem;
    sal_Int16       mnCommand;

    EffectSequenceHelper* mpEffectSequence;

    String          maName;

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxNode;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAudio > mxAudio;
    ::com::sun::star::uno::Any maTarget;

    sal_Bool        mbHasAfterEffect;
    ::com::sun::star::uno::Any maDimColor;
    sal_Int32       mnMasterRel;
};

struct stl_CustomAnimationEffect_search_node_predict
{
    stl_CustomAnimationEffect_search_node_predict( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xSearchNode );
    bool operator()( CustomAnimationEffectPtr pEffect ) const;
    const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& mxSearchNode;
};

enum ESequenceHint { EFFECT_EDITED, EFFECT_REMOVED, EFFECT_ADDED };

/** this listener is implemented by UI components to track changes in the animation core */
class ISequenceListener
{
public:
    virtual void notify_change() = 0;
};

/** this class keeps track of a group of animations that build up
    a text animation for a single shape */
class CustomAnimationTextGroup
{
    friend class EffectSequenceHelper;

public:
    CustomAnimationTextGroup( const ::com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rTarget, sal_Int32 nGroupId );

    void reset();
    void addEffect( CustomAnimationEffectPtr& pEffect );

    const ::com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& getTarget() const { return maTarget; }
    const EffectSequence& getEffects() const { return maEffects; }

    /* -1: as single object, 0: all at once, n > 0: by n Th paragraph */
    sal_Int32 getTextGrouping() const { return mnTextGrouping; }

    sal_Bool getAnimateForm() const { return mbAnimateForm; }
    sal_Bool getTextReverse() const { return mbTextReverse; }
    double getTextGroupingAuto() const { return mfGroupingAuto; }

private:
    EffectSequence maEffects;
    ::com::sun::star::uno::Reference< com::sun::star::drawing::XShape > maTarget;

    sal_Int32 mnTextGrouping;
    sal_Bool mbAnimateForm;
    sal_Bool mbTextReverse;
    double mfGroupingAuto;
    sal_Int32 mnLastPara;
    sal_Int8 mnDepthFlags[5];
    sal_Int32 mnGroupId;
};

typedef boost::shared_ptr< CustomAnimationTextGroup > CustomAnimationTextGroupPtr;
typedef std::map< sal_Int32, CustomAnimationTextGroupPtr > CustomAnimationTextGroupMap;

class EffectSequenceHelper
{
friend class MainSequence;

public:
    EffectSequenceHelper();
    EffectSequenceHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer >& xSequenceRoot );
    virtual ~EffectSequenceHelper();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getRootNode();

    CustomAnimationEffectPtr append( const CustomAnimationPresetPtr& pDescriptor, const ::com::sun::star::uno::Any& rTarget, double fDuration = -1.0 );
    void append( const CustomAnimationEffectPtr& pEffect );
    void insert( EffectSequence::iterator& rPos, const CustomAnimationEffectPtr& pEffect );
    void replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pDescriptor, double fDuration = -1.0 );
    void replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pDescriptor, const rtl::OUString& rPresetSubType, double fDuration = -1.0 );
    void remove( const CustomAnimationEffectPtr& pEffect );

    void create( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void createEffectsequence( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void processAfterEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void createEffects( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    virtual bool disposeShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    virtual void insertTextRange( const com::sun::star::uno::Any& aTarget );
    virtual void disposeTextRange( const com::sun::star::uno::Any& aTarget );
    virtual bool hasEffect( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    virtual void onTextChanged( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );

    /** this must be called if effects from this sequence are changed.
        the method will call the registered listeners */
    void update( const CustomAnimationEffectPtr& pEffect );

    /** this method rebuilds the animation nodes */
    virtual void rebuild();

    EffectSequence::iterator getBegin() { return maEffects.begin(); }
    EffectSequence::iterator getEnd() { return maEffects.end(); }
    EffectSequence::iterator find( const CustomAnimationEffectPtr& pEffect );

    EffectSequence& getSequence() { return maEffects; }

    void addListener( ISequenceListener* pListener );
    void removeListener( ISequenceListener* pListener );

    // text group methods

    CustomAnimationTextGroupPtr findGroup( sal_Int32 nGroupId );
    CustomAnimationTextGroupPtr createTextGroup( CustomAnimationEffectPtr pEffect, sal_Int32 nTextGrouping, double fTextGroupingAuto, sal_Bool bAnimateForm, sal_Bool bTextReverse );
    void setTextGrouping( CustomAnimationTextGroupPtr pTextGroup, sal_Int32 nTextGrouping );
    void setAnimateForm( CustomAnimationTextGroupPtr pTextGroup, sal_Bool bAnimateForm );
    void setTextGroupingAuto( CustomAnimationTextGroupPtr pTextGroup, double fTextGroupingAuto );
    void setTextReverse( CustomAnimationTextGroupPtr pTextGroup, sal_Bool bAnimateForm );

    sal_Int32 getSequenceType() const { return mnSequenceType; }

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTriggerShape() const { return mxEventSource; }
    void setTriggerShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xTrigger ) { mxEventSource = xTrigger; }

protected:
    virtual void implRebuild();
    virtual void reset();

    void createTextGroupParagraphEffects( CustomAnimationTextGroupPtr pTextGroup, CustomAnimationEffectPtr pEffect, bool bUsed );

    void notify_listeners();

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer > createParallelTimeContainer() const;

    void updateTextGroups();

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer > mxSequenceRoot;
    EffectSequence maEffects;
    std::list< ISequenceListener* > maListeners;
    CustomAnimationTextGroupMap maGroupMap;
    sal_Int32 mnSequenceType;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxEventSource;
};

class MainSequence;

class InteractiveSequence : public EffectSequenceHelper
{
friend class MainSequence;

public:
    InteractiveSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer >& xSequenceRoot, MainSequence* pMainSequence );

    /** this method rebuilds the animation nodes */
    virtual void rebuild();

private:
    virtual void implRebuild();

    MainSequence*   mpMainSequence;
};

typedef boost::shared_ptr< InteractiveSequence > InteractiveSequencePtr;
typedef std::list< InteractiveSequencePtr > InteractiveSequenceList;

class MainSequence : public EffectSequenceHelper, public ISequenceListener
{
    friend class UndoAnimation;
    friend class MainSequenceRebuildGuard;

public:
    MainSequence();
    MainSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xTimingRootNode );
    ~MainSequence();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getRootNode();
    void reset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xTimingRootNode );

    /** this method rebuilds the animation nodes */
    virtual void rebuild();

    virtual bool disposeShape( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    virtual void insertTextRange( const com::sun::star::uno::Any& aTarget );
    virtual void disposeTextRange( const com::sun::star::uno::Any& aTarget );
    virtual bool hasEffect( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );
    virtual void onTextChanged( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );

    const InteractiveSequenceList& getInteractiveSequenceList() const { return maInteractiveSequenceList; }

    virtual void notify_change();

    bool setTrigger( const CustomAnimationEffectPtr& pEffect, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xTriggerShape );

    /** starts a timer that recreates the internal structure from the API core after 1 second */
    void startRecreateTimer();

    /** starts a timer that rebuilds the API core from the internal structure after 1 second */
    void startRebuildTimer();

protected:
    /** permits rebuilds until unlockRebuilds() is called. All rebuild calls during a locked sequence are
        process after unlockRebuilds() call. lockRebuilds() and unlockRebuilds() calls can be nested. */
    void lockRebuilds();
    void unlockRebuilds();

    DECL_LINK( onTimerHdl, Timer * );

    virtual void implRebuild();

    void init();

    void create();
    virtual void reset();

    InteractiveSequencePtr createInteractiveSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );

    InteractiveSequenceList maInteractiveSequenceList;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesListener > mxChangesListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer > mxTimingRootNode;
    Timer maTimer;
    bool mbTimerMode;
    bool mbRebuilding;

    long mnRebuildLockGuard;
    bool mbPendingRebuildRequest;
};

typedef boost::shared_ptr< MainSequence > MainSequencePtr;

class MainSequenceRebuildGuard
{
public:
    MainSequenceRebuildGuard( const MainSequencePtr& pMainSequence );
    ~MainSequenceRebuildGuard();

private:
    MainSequencePtr mpMainSequence;
};

}

#endif // _SD_CUSTOMANIMATIONEFFECT_HXX
