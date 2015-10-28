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

#ifndef INCLUDED_SD_INC_CUSTOMANIMATIONEFFECT_HXX
#define INCLUDED_SD_INC_CUSTOMANIMATIONEFFECT_HXX

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/util/XChangesListener.hpp>

#include <vcl/timer.hxx>

#include <sddllapi.h>

#include <list>
#include <map>
#include <memory>

class SdrPathObj;

namespace sd {

enum EValue { VALUE_FROM, VALUE_TO, VALUE_BY, VALUE_FIRST, VALUE_LAST };

class CustomAnimationEffect;

class CustomAnimationPreset;
typedef std::shared_ptr< CustomAnimationPreset > CustomAnimationPresetPtr;

typedef std::shared_ptr< CustomAnimationEffect > CustomAnimationEffectPtr;

typedef std::list< CustomAnimationEffectPtr > EffectSequence;

class EffectSequenceHelper;

class SD_DLLPUBLIC CustomAnimationEffect
{
    friend class MainSequence;
    friend class EffectSequenceHelper;

public:
    CustomAnimationEffect( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    virtual ~CustomAnimationEffect();

    SAL_DLLPRIVATE const css::uno::Reference< css::animations::XAnimationNode >& getNode() const { return mxNode; }
    SAL_DLLPRIVATE void setNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    SAL_DLLPRIVATE void replaceNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    SAL_DLLPRIVATE CustomAnimationEffectPtr clone() const;

    // attributes
    SAL_DLLPRIVATE const OUString&    getPresetId() const { return maPresetId; }
    SAL_DLLPRIVATE const OUString&    getPresetSubType() const { return maPresetSubType; }
    SAL_DLLPRIVATE const OUString&    getProperty() const { return maProperty; }

    SAL_DLLPRIVATE sal_Int16       getPresetClass() const { return mnPresetClass; }
    SAL_DLLPRIVATE void            setPresetClass( sal_Int16 nPresetClass );

    SAL_DLLPRIVATE sal_Int16       getNodeType() const { return mnNodeType; }
    void                           setNodeType( sal_Int16 nNodeType );

    SAL_DLLPRIVATE css::uno::Any   getRepeatCount() const;
    SAL_DLLPRIVATE void            setRepeatCount( const css::uno::Any& rRepeatCount );

    SAL_DLLPRIVATE css::uno::Any   getEnd() const;
    SAL_DLLPRIVATE void            setEnd( const css::uno::Any& rEnd );

    SAL_DLLPRIVATE sal_Int16       getFill() const;
    SAL_DLLPRIVATE void            setFill( sal_Int16 nFill );

    SAL_DLLPRIVATE double          getBegin() const { return mfBegin; }
    void                           setBegin( double fBegin );

    SAL_DLLPRIVATE double          getDuration() const { return mfDuration; }
    void                           setDuration( double fDuration );

    SAL_DLLPRIVATE double          getAbsoluteDuration() const { return mfAbsoluteDuration; }

    SAL_DLLPRIVATE sal_Int16       getIterateType() const { return mnIterateType; }
    void                           setIterateType( sal_Int16 nIterateType );

    SAL_DLLPRIVATE double          getIterateInterval() const { return mfIterateInterval; }
    void                           setIterateInterval( double fIterateInterval );

    SAL_DLLPRIVATE css::uno::Any  getTarget() const { return maTarget; }
    void                          setTarget( const css::uno::Any& rTarget );

    SAL_DLLPRIVATE bool             hasAfterEffect() const { return mbHasAfterEffect; }
    SAL_DLLPRIVATE void            setHasAfterEffect( bool bHasAfterEffect ) { mbHasAfterEffect = bHasAfterEffect; }

    SAL_DLLPRIVATE css::uno::Any   getDimColor() const { return maDimColor; }
    SAL_DLLPRIVATE void            setDimColor( const css::uno::Any& rDimColor ) { maDimColor = rDimColor; }

    SAL_DLLPRIVATE bool            IsAfterEffectOnNext() const { return mbAfterEffectOnNextEffect; }
    SAL_DLLPRIVATE void            setAfterEffectOnNext( bool bOnNextEffect ) { mbAfterEffectOnNextEffect = bOnNextEffect; }

    SAL_DLLPRIVATE sal_Int32       getParaDepth() const { return mnParaDepth; }

    SAL_DLLPRIVATE bool            hasText() const { return mbHasText; }

    SAL_DLLPRIVATE sal_Int16       getCommand() const { return mnCommand; }

    SAL_DLLPRIVATE double          getAcceleration() const { return mfAcceleration; }
    SAL_DLLPRIVATE void            setAcceleration( double fAcceleration );

    SAL_DLLPRIVATE double          getDecelerate() const { return mfDecelerate; }
    SAL_DLLPRIVATE void            setDecelerate( double fDecelerate );

    SAL_DLLPRIVATE bool            getAutoReverse() const { return mbAutoReverse; }
    SAL_DLLPRIVATE void            setAutoReverse( bool bAutoReverse );

    SAL_DLLPRIVATE css::uno::Any  getProperty( sal_Int32 nNodeType, const OUString& rAttributeName, EValue eValue );
    SAL_DLLPRIVATE bool           setProperty( sal_Int32 nNodeType, const OUString& rAttributeName, EValue eValue, const css::uno::Any& rValue );

    SAL_DLLPRIVATE css::uno::Any  getTransformationProperty( sal_Int32 nTransformType, EValue eValue );
    SAL_DLLPRIVATE bool           setTransformationProperty( sal_Int32 nTransformType, EValue eValue, const css::uno::Any& rValue );

    SAL_DLLPRIVATE css::uno::Any  getColor( sal_Int32 nIndex );
    SAL_DLLPRIVATE void            setColor( sal_Int32 nIndex, const css::uno::Any& rColor );

    SAL_DLLPRIVATE sal_Int32       getGroupId() const { return mnGroupId; }
    SAL_DLLPRIVATE void            setGroupId( sal_Int32 nGroupId );

    SAL_DLLPRIVATE sal_Int16       getTargetSubItem() const { return mnTargetSubItem; }
    void                           setTargetSubItem( sal_Int16 nSubItem );

    SAL_DLLPRIVATE OUString getPath() const;
    SAL_DLLPRIVATE void setPath( const OUString& rPath );

    SAL_DLLPRIVATE bool checkForText();
    SAL_DLLPRIVATE bool calculateIterateDuration();

    SAL_DLLPRIVATE void setAudio( const css::uno::Reference< css::animations::XAudio >& xAudio );
    SAL_DLLPRIVATE bool getStopAudio() const;
    void setStopAudio();
    void createAudio( const css::uno::Any& rSource, double fVolume = 1.0 );
    SAL_DLLPRIVATE void removeAudio();
    SAL_DLLPRIVATE const css::uno::Reference< css::animations::XAudio >& getAudio() const { return mxAudio; }

    SAL_DLLPRIVATE EffectSequenceHelper*   getEffectSequence() const { return mpEffectSequence; }

    // helper
    SAL_DLLPRIVATE css::uno::Reference< css::animations::XAnimationNode > createAfterEffectNode() const throw (css::uno::Exception);
    SAL_DLLPRIVATE css::uno::Reference< css::drawing::XShape > getTargetShape() const;

    // static helpers
    SAL_DLLPRIVATE static sal_Int32 get_node_type( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    SAL_DLLPRIVATE static sal_Int32 getNumberOfSubitems( const css::uno::Any& aTarget, sal_Int16 nIterateType );

    SAL_DLLPRIVATE SdrPathObj* createSdrPathObjFromPath();
    SAL_DLLPRIVATE void updateSdrPathObjFromPath( SdrPathObj& rPathObj );
    SAL_DLLPRIVATE void updatePathFromSdrPathObj( const SdrPathObj& rPathObj );

protected:
    SAL_DLLPRIVATE void setEffectSequence( EffectSequenceHelper* pSequence ) { mpEffectSequence = pSequence; }

private:
    sal_Int16       mnNodeType;
    OUString        maPresetId;
    OUString        maPresetSubType;
    OUString        maProperty;
    sal_Int16       mnPresetClass;
    double          mfBegin;
    double          mfDuration;                 // this is the maximum duration of the subeffects
    double          mfAbsoluteDuration;         // this is the maximum duration of the subeffects including possible iterations
    sal_Int32       mnGroupId;
    sal_Int16       mnIterateType;
    double          mfIterateInterval;
    sal_Int32       mnParaDepth;
    bool            mbHasText;
    double          mfAcceleration;
    double          mfDecelerate;
    bool            mbAutoReverse;
    sal_Int16       mnTargetSubItem;
    sal_Int16       mnCommand;

    EffectSequenceHelper* mpEffectSequence;

    css::uno::Reference< css::animations::XAnimationNode > mxNode;
    css::uno::Reference< css::animations::XAudio > mxAudio;
    css::uno::Any maTarget;

    bool        mbHasAfterEffect;
    css::uno::Any maDimColor;
    bool        mbAfterEffectOnNextEffect;
};

struct stl_CustomAnimationEffect_search_node_predict
{
    stl_CustomAnimationEffect_search_node_predict( const css::uno::Reference< css::animations::XAnimationNode >& xSearchNode );
    bool operator()( CustomAnimationEffectPtr pEffect ) const;
    const css::uno::Reference< css::animations::XAnimationNode >& mxSearchNode;
};

enum ESequenceHint { EFFECT_EDITED, EFFECT_REMOVED, EFFECT_ADDED };

/** this listener is implemented by UI components to track changes in the animation core */
class ISequenceListener
{
public:
    virtual void notify_change() = 0;

protected:
    ~ISequenceListener() {}
};

/** this class keeps track of a group of animations that build up
    a text animation for a single shape */
class CustomAnimationTextGroup
{
    friend class EffectSequenceHelper;

public:
    CustomAnimationTextGroup( const css::uno::Reference< css::drawing::XShape >& rTarget, sal_Int32 nGroupId );

    void reset();
    void addEffect( CustomAnimationEffectPtr& pEffect );

    const EffectSequence& getEffects() const { return maEffects; }

    /* -1: as single object, 0: all at once, n > 0: by n Th paragraph */
    sal_Int32 getTextGrouping() const { return mnTextGrouping; }

    bool getAnimateForm() const { return mbAnimateForm; }
    bool getTextReverse() const { return mbTextReverse; }
    double getTextGroupingAuto() const { return mfGroupingAuto; }

private:
    EffectSequence maEffects;
    css::uno::Reference< css::drawing::XShape > maTarget;

    enum { PARA_LEVELS = 5 };

    sal_Int32 mnTextGrouping;
    bool mbAnimateForm;
    bool mbTextReverse;
    double mfGroupingAuto;
    sal_Int32 mnLastPara;
    sal_Int8 mnDepthFlags[PARA_LEVELS];
    sal_Int32 mnGroupId;
};

typedef std::shared_ptr< CustomAnimationTextGroup > CustomAnimationTextGroupPtr;
typedef std::map< sal_Int32, CustomAnimationTextGroupPtr > CustomAnimationTextGroupMap;

class SD_DLLPUBLIC EffectSequenceHelper
{
friend class MainSequence;

public:
    SAL_DLLPRIVATE EffectSequenceHelper();
    SAL_DLLPRIVATE EffectSequenceHelper( const css::uno::Reference< css::animations::XTimeContainer >& xSequenceRoot );
    SAL_DLLPRIVATE virtual ~EffectSequenceHelper();

    SAL_DLLPRIVATE virtual css::uno::Reference< css::animations::XAnimationNode > getRootNode();

    SAL_DLLPRIVATE CustomAnimationEffectPtr append( const CustomAnimationPresetPtr& pDescriptor, const css::uno::Any& rTarget, double fDuration = -1.0 );
    SAL_DLLPRIVATE CustomAnimationEffectPtr append( const SdrPathObj& rPathObj, const css::uno::Any& rTarget, double fDuration = -1.0 );
    void append( const CustomAnimationEffectPtr& pEffect );
    SAL_DLLPRIVATE void replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pDescriptor, double fDuration = -1.0 );
    SAL_DLLPRIVATE void replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pDescriptor, const OUString& rPresetSubType, double fDuration = -1.0 );
    SAL_DLLPRIVATE void remove( const CustomAnimationEffectPtr& pEffect );

    SAL_DLLPRIVATE void create( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    SAL_DLLPRIVATE void createEffectsequence( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    SAL_DLLPRIVATE void processAfterEffect( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    SAL_DLLPRIVATE void createEffects( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    SAL_DLLPRIVATE sal_Int32 getCount() const { return sal::static_int_cast< sal_Int32 >( maEffects.size() ); }

    SAL_DLLPRIVATE virtual CustomAnimationEffectPtr findEffect( const css::uno::Reference< css::animations::XAnimationNode >& xNode ) const;

    SAL_DLLPRIVATE virtual bool disposeShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    SAL_DLLPRIVATE virtual void insertTextRange( const css::uno::Any& aTarget );
    SAL_DLLPRIVATE virtual void disposeTextRange( const css::uno::Any& aTarget );
    SAL_DLLPRIVATE virtual bool hasEffect( const css::uno::Reference< css::drawing::XShape >& xShape );
    SAL_DLLPRIVATE virtual void onTextChanged( const css::uno::Reference< css::drawing::XShape >& xShape );

    /** this method rebuilds the animation nodes */
    SAL_DLLPRIVATE virtual void rebuild();

    SAL_DLLPRIVATE EffectSequence::iterator getBegin() { return maEffects.begin(); }
    SAL_DLLPRIVATE EffectSequence::iterator getEnd() { return maEffects.end(); }
    SAL_DLLPRIVATE EffectSequence::iterator find( const CustomAnimationEffectPtr& pEffect );

    SAL_DLLPRIVATE EffectSequence& getSequence() { return maEffects; }

    SAL_DLLPRIVATE void addListener( ISequenceListener* pListener );
    SAL_DLLPRIVATE void removeListener( ISequenceListener* pListener );

    // text group methods

    SAL_DLLPRIVATE CustomAnimationTextGroupPtr findGroup( sal_Int32 nGroupId );
    CustomAnimationTextGroupPtr    createTextGroup( CustomAnimationEffectPtr pEffect, sal_Int32 nTextGrouping, double fTextGroupingAuto, bool bAnimateForm, bool bTextReverse );
    SAL_DLLPRIVATE void setTextGrouping( CustomAnimationTextGroupPtr pTextGroup, sal_Int32 nTextGrouping );
    SAL_DLLPRIVATE void setAnimateForm( CustomAnimationTextGroupPtr pTextGroup, bool bAnimateForm );
    SAL_DLLPRIVATE void setTextGroupingAuto( CustomAnimationTextGroupPtr pTextGroup, double fTextGroupingAuto );
    SAL_DLLPRIVATE void setTextReverse( CustomAnimationTextGroupPtr pTextGroup, bool bAnimateForm );

    SAL_DLLPRIVATE sal_Int32 getSequenceType() const { return mnSequenceType; }

    SAL_DLLPRIVATE css::uno::Reference< css::drawing::XShape > getTriggerShape() const { return mxEventSource; }
    SAL_DLLPRIVATE void setTriggerShape( const css::uno::Reference< css::drawing::XShape >& xTrigger ) { mxEventSource = xTrigger; }

    SAL_DLLPRIVATE virtual sal_Int32 getOffsetFromEffect( const CustomAnimationEffectPtr& xEffect ) const;
    SAL_DLLPRIVATE virtual CustomAnimationEffectPtr getEffectFromOffset( sal_Int32 nOffset ) const;

protected:
    SAL_DLLPRIVATE virtual void implRebuild();
    SAL_DLLPRIVATE virtual void reset();

    SAL_DLLPRIVATE void createTextGroupParagraphEffects( CustomAnimationTextGroupPtr pTextGroup, CustomAnimationEffectPtr pEffect, bool bUsed );

    SAL_DLLPRIVATE void notify_listeners();

    SAL_DLLPRIVATE void updateTextGroups();

protected:
    css::uno::Reference< css::animations::XTimeContainer > mxSequenceRoot;
    EffectSequence maEffects;
    std::list< ISequenceListener* > maListeners;
    CustomAnimationTextGroupMap maGroupMap;
    sal_Int32 mnSequenceType;
    css::uno::Reference< css::drawing::XShape > mxEventSource;
};

class MainSequence;

class InteractiveSequence : public EffectSequenceHelper
{
friend class MainSequence;
friend class MainSequenceChangeGuard;

public:
    InteractiveSequence( const css::uno::Reference< css::animations::XTimeContainer >& xSequenceRoot, MainSequence* pMainSequence );

    /** this method rebuilds the animation nodes */
    virtual void rebuild() override;

private:
    virtual void implRebuild() override;

    MainSequence*   mpMainSequence;
};

typedef std::shared_ptr< InteractiveSequence > InteractiveSequencePtr;
typedef std::list< InteractiveSequencePtr > InteractiveSequenceList;

class MainSequence : public EffectSequenceHelper, public ISequenceListener
{
    friend class UndoAnimation;
    friend class MainSequenceRebuildGuard;
    friend class MainSequenceChangeGuard;

public:
    MainSequence();
    MainSequence( const css::uno::Reference< css::animations::XAnimationNode >& xTimingRootNode );
    virtual ~MainSequence();

    virtual css::uno::Reference< css::animations::XAnimationNode > getRootNode() override;
    void reset( const css::uno::Reference< css::animations::XAnimationNode >& xTimingRootNode );

    /** this method rebuilds the animation nodes */
    virtual void rebuild() override;

    virtual CustomAnimationEffectPtr findEffect( const css::uno::Reference< css::animations::XAnimationNode >& xNode ) const override;

    virtual bool disposeShape( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void insertTextRange( const css::uno::Any& aTarget ) override;
    virtual void disposeTextRange( const css::uno::Any& aTarget ) override;
    virtual bool hasEffect( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void onTextChanged( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    const InteractiveSequenceList& getInteractiveSequenceList() const { return maInteractiveSequenceList; }

    virtual void notify_change() override;

    bool setTrigger( const CustomAnimationEffectPtr& pEffect, const css::uno::Reference< css::drawing::XShape >& xTriggerShape );

    /** starts a timer that recreates the internal structure from the API core after 1 second */
    void startRecreateTimer();

    /** starts a timer that rebuilds the API core from the internal structure after 1 second */
    void startRebuildTimer();

    virtual sal_Int32 getOffsetFromEffect( const CustomAnimationEffectPtr& xEffect ) const override;
    virtual CustomAnimationEffectPtr getEffectFromOffset( sal_Int32 nOffset ) const override;

protected:
    /** permits rebuilds until unlockRebuilds() is called. All rebuild calls during a locked sequence are
        process after unlockRebuilds() call. lockRebuilds() and unlockRebuilds() calls can be nested. */
    void lockRebuilds();
    void unlockRebuilds();

    DECL_LINK_TYPED(onTimerHdl, Timer *, void);

    virtual void implRebuild() override;

    void init();

    void createMainSequence();
    virtual void reset() override;

    InteractiveSequencePtr createInteractiveSequence( const css::uno::Reference< css::drawing::XShape >& xShape );

    InteractiveSequenceList maInteractiveSequenceList;

    css::uno::Reference< css::util::XChangesListener > mxChangesListener;
    css::uno::Reference< css::animations::XTimeContainer > mxTimingRootNode;
    Timer maTimer;
    bool mbTimerMode;
    bool mbRebuilding;

    long mnRebuildLockGuard;
    bool mbPendingRebuildRequest;
    sal_Int32 mbIgnoreChanges;
};

typedef std::shared_ptr< MainSequence > MainSequencePtr;

class MainSequenceRebuildGuard
{
public:
    MainSequenceRebuildGuard( const MainSequencePtr& pMainSequence );
    ~MainSequenceRebuildGuard();

private:
    MainSequencePtr mpMainSequence;
};

}

#endif // INCLUDED_SD_INC_CUSTOMANIMATIONEFFECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
