/*************************************************************************
 *
 *  $RCSfile: filtergrouping.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-10-02 12:37:54 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SFX2_FILTERGROUPING_HXX
#include "filtergrouping.hxx"
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include "filedlghelper.hxx"
#endif
#ifndef _SFX_HRC
#include "sfx.hrc"
#endif
#ifndef _SFX_SFXRESID_HXX
#include "sfxresid.hxx"
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERGROUPMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_STRINGPAIR_HPP_
#include <com/sun/star/beans/StringPair.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _LIST_
#include <list>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _MAP_
#include <map>
#endif
#ifndef _ALGORITHM_
#include <algorithm>
#endif

//........................................................................
namespace sfx2
{
//........................................................................

#define DISABLE_GROUPING_AND_CLASSIFYING
    // not using the functionallity herein, yet

#ifdef FS_PRIV_DEGUG
#undef DISABLE_GROUPING_AND_CLASSIFYING
    // but enable it for FS' own workenvironment
#endif

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::utl;

    //====================================================================
    /**

    Some general words about what's going on here ....

    <p>In our file open dialog, usually we display every filter we know. That's how it was before: every filter
    lead to an own line in the filter list box, e.g. "StarWriter 5.0 Dokument" or "Microsoft Word 97".</p>

    <p>But then the PM came. And everything changed ....</p>

    <p>A basic idea are groups: Why simply listing all the single filters? Couldn't we draw nice separators
    between the filters which logically belong together? I.e. all the filters which open a document in StarWriter:
    couldn't we separate them from all the filters which open the document in StarCalc?<br/>
    So spoke the PM, and engineering obeyed.</p>

    <p>So we have groups. They're just a visual aspect: All the filters of a group are presented together, separated
    by a line from other groups.</p>

    <p>Let's be honest: How the concrete implementation of the file picker service separates the different groups
    is a matter of this implementation. We only do this grouping and suggest it to the FilePicker service ...</p>

    <p>Now for the second concept:<br/>
    Thinking about it (and that's what the PM did), both "StarWriter 5.0 Dokument" and "Microsoft Word 97"
    describe a text document. It's a text. It's of no interest for the user that one of the texts was saved in
    MS' format, and one in our own format.<br/>
    So in a first step, we want to have a filter entry "Text documents". This would cover both above-mentioned
    filters, as well as any other filters for documents which are texts.</p>

    <p>Such an entry as "Text documents" is - within the scope of this file - called "class" or "filter class".</p>

    <p>In the file-open-dialog, such a class looks like an ordinary filter: it's simply a name in the filter
    listbox. Selecting means that all the files matching one of the "sub-filters" are displayed (in the example above,
    this would be "*.sdw", "*.doc" and so on).</p>

    <p>Now the are two types of filter classes: global ones and local ones. "Text documents" is a global class. As
    well as "Spreadsheets". Or "Web pages".<br/>
    Let's have a look at a local class: The filters "MS Word 95" and "MS WinWord 6.0" together form the class
    "Microsoft Word 6.0 / 95" (don't ask for the reasons. At least not me. Ask the PM). There are a lot of such
    local classes ...</p>

    <p>The difference between global and local classes is as follows: Global classes are presented in an own group.
    There is one dedicated group at the top of the list, containing all the global groups - no local groups and no
    single filters.</p>

    <p>Ehm - it was a lie. Not really at the top. Before this group, there is this single "All files" entry. It forms
    it's own group. But this is uninteresting here.</p>

    <p>Local classes must consist of filters which - without the classification - would all belong to the same group.
    Then, they're combined to one entry (in the example above: "Microsoft Word 6.0 / 95"), and this entry is inserted
    into the file picker filter list, instead of the single filters which form the class.</p>

    <p>This is an interesting difference between local and global classes: Filters which are part of a global class
    are listed in there own group, too. Filters in local classes aren't listed a second time - neither directly (as
    the filter itself) nor indirectly (as part of another local group).</p>

    <p>The only exception are filters which are part of a global class <em>and</em> a local class. This is allowed.
    Beeing cotained in two local classes isn't.</p>

    <p>So that's all what you need to know: Understand the concept of "filter classes" (a filter class combines
    different filters and acts as if it's a filter itself) and the concept of groups (a group just describes a
    logical correlation of filters and usually is represented to the user by drawing group separators in the filter
    list).</p>

    <p>If you got it, go try understanding this file :).</p>
    */


    //====================================================================

    typedef StringPair                          FilterDescriptor;   // a single filter or a filter class (display name and filter mask)
    typedef ::std::list< FilterDescriptor >     FilterGroup;            // a list of single filter entries
    typedef ::std::list< FilterGroup >          GroupedFilterList;  // a list of all filters, already grouped

    /// the logical name of a filter
    typedef ::rtl::OUString                     FilterName;

    /// a descriptor for a filter class (which in the final dialog is represented by one filter entry)
    typedef struct _tagFilterClass
    {
        ::rtl::OUString             sDisplayName;       // the display name
        Sequence< FilterName >      aSubFilters;        // the (logical) names of the filter which belong to the class
    } FilterClass;

    typedef ::std::list< FilterClass >      FilterClassList;

    //====================================================================

    //--------------------------------------------------------------------
    void lcl_ReadFilterClass( const OConfigurationNode& _rClassesNode, const ::rtl::OUString& _rLogicalClassName,
        FilterClass& /* [out] */ _rClass )
    {
        static const ::rtl::OUString sDisplaNameNodeName = ::rtl::OUString::createFromAscii( "DisplayName" );
        static const ::rtl::OUString sSubFiltersNodeName = ::rtl::OUString::createFromAscii( "Filters" );

            // the description node for the current class
        OConfigurationNode aClassDesc = _rClassesNode.openNode( _rLogicalClassName );

        // the values
        aClassDesc.getNodeValue( sDisplaNameNodeName ) >>= _rClass.sDisplayName;
        aClassDesc.getNodeValue( sSubFiltersNodeName ) >>= _rClass.aSubFilters;
    }

    //--------------------------------------------------------------------
    typedef ::std::map< ::rtl::OUString, FilterClassList::iterator > FilterClassReferrer;

    //--------------------------------------------------------------------
    struct CreateEmptyClassRememberPos : public ::std::unary_function< FilterName, void >
    {
    protected:
        FilterClassList&        m_rClassList;
        FilterClassReferrer&    m_rClassesReferrer;

    public:
        CreateEmptyClassRememberPos( FilterClassList& _rClassList, FilterClassReferrer& _rClassesReferrer )
            :m_rClassList       ( _rClassList )
            ,m_rClassesReferrer ( _rClassesReferrer )
        {
        }

        // operate on a single class name
        void operator() ( const FilterName& _rLogicalFilterName )
        {
            // insert a new (empty) class
            m_rClassList.push_back( FilterClass() );
            // get the position of this new entry
            FilterClassList::iterator aInsertPos = m_rClassList.end();
            --aInsertPos;
            // remember this position
            m_rClassesReferrer.insert( FilterClassReferrer::value_type( _rLogicalFilterName, aInsertPos ) );
        }
    };

    //--------------------------------------------------------------------
    struct ReadGlobalFilter : public ::std::unary_function< FilterName, void >
    {
    protected:
        OConfigurationNode      m_aClassesNode;
        FilterClassReferrer&    m_aClassReferrer;

    public:
        ReadGlobalFilter( const OConfigurationNode& _rClassesNode, FilterClassReferrer& _rClassesReferrer )
            :m_aClassesNode     ( _rClassesNode )
            ,m_aClassReferrer   ( _rClassesReferrer )
        {
        }

        // operate on a single logical name
        void operator() ( const FilterName& _rName )
        {
            FilterClassReferrer::iterator aClassRef = m_aClassReferrer.find( _rName );
            if ( m_aClassReferrer.end() == aClassRef )
            {
                // we do not know this global class
                DBG_ERROR( "ReadGlobalFilter::operator(): unknown filter name!" );
                // TODO: perhaps we should be more tolerant - at the moment, the filter is dropped
                // We could silently push_back it to the container ....
            }
            else
            {
                // read the data of this class into the node referred to by aClassRef
                lcl_ReadFilterClass( m_aClassesNode, _rName, *aClassRef->second );
            }
        }
    };

    //--------------------------------------------------------------------
    void lcl_ReadGlobalFilters( const OConfigurationNode& _rFilterClassification, FilterClassList& _rGlobalClasses )
    {
        _rGlobalClasses.clear();

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // get the list describing the order of all global classes
        Sequence< ::rtl::OUString > aGlobalClasses;
        _rFilterClassification.getNodeValue( ::rtl::OUString::createFromAscii( "GlobalFilters/Order" ) ) >>= aGlobalClasses;

        // Global classes are presented in an own group, so their order matters (while the order of the
        // "local classes" doesn't).
        // That's why we can't simply add the global classes to _rGlobalClasses using the order in which they
        // are returned from the configuration - it is completely undefined, and we need a _defined_ order.
        FilterClassReferrer aClassReferrer;
        ::std::for_each(
            aGlobalClasses.getConstArray(),
            aGlobalClasses.getConstArray() + aGlobalClasses.getLength(),
            CreateEmptyClassRememberPos( _rGlobalClasses, aClassReferrer )
        );
            // now _rGlobalClasses contains a dummy entry for each global class,
            // while aClassReferrer maps from the logical name of the class to the position within _rGlobalClasses where
            // it's dummy entry resides

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // go for all the single class entries
        OConfigurationNode aFilterClassesNode = _rFilterClassification.openNode( ::rtl::OUString::createFromAscii( "GlobalFilters/Classes" ) );
        Sequence< ::rtl::OUString > aFilterClasses = aFilterClassesNode.getNodeNames();
        ::std::for_each(
            aFilterClasses.getConstArray(),
            aFilterClasses.getConstArray() + aFilterClasses.getLength(),
            ReadGlobalFilter( aFilterClassesNode, aClassReferrer )
        );
    }

    //--------------------------------------------------------------------
    struct ReadLocalFilter : public ::std::unary_function< FilterName, void >
    {
    protected:
        OConfigurationNode      m_aClassesNode;
        FilterClassList&        m_rClasses;

    public:
        ReadLocalFilter( const OConfigurationNode& _rClassesNode, FilterClassList& _rClasses )
            :m_aClassesNode ( _rClassesNode )
            ,m_rClasses     ( _rClasses )
        {
        }

        // operate on a single logical name
        void operator() ( const FilterName& _rName )
        {
            // read the data for this class
            FilterClass aClass;
            lcl_ReadFilterClass( m_aClassesNode, _rName, aClass );

            // insert the class descriptor
            m_rClasses.push_back( aClass );
        }
    };

    //--------------------------------------------------------------------
    void lcl_ReadLocalFilters( const OConfigurationNode& _rFilterClassification, FilterClassList& _rLocalClasses )
    {
        _rLocalClasses.clear();

        // the node for the local classes
        OConfigurationNode aFilterClassesNode = _rFilterClassification.openNode( ::rtl::OUString::createFromAscii( "LocalFilters/Classes" ) );
        Sequence< ::rtl::OUString > aFilterClasses = aFilterClassesNode.getNodeNames();

        ::std::for_each(
            aFilterClasses.getConstArray(),
            aFilterClasses.getConstArray() + aFilterClasses.getLength(),
            ReadLocalFilter( aFilterClassesNode, _rLocalClasses )
        );
    }

    //--------------------------------------------------------------------
    void lcl_ReadClassification( FilterClassList& _rGlobalClasses, FilterClassList& _rLocalClasses )
    {
        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // open our config node
        OConfigurationTreeRoot aFilterClassification = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(),
            ::rtl::OUString::createFromAscii( "org.openoffice.Office.UI/FilterClassification" ),
            -1,
            OConfigurationTreeRoot::CM_READONLY
        );

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // go for the global classes
        lcl_ReadGlobalFilters( aFilterClassification, _rGlobalClasses );

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // fo for the local classes
        lcl_ReadLocalFilters( aFilterClassification, _rLocalClasses );

    }

    //--------------------------------------------------------------------
    // a struct which holds references from a logical filter name to a filter class entry
    // used quick lookup of classes which a given filter may belong to
    typedef ::std::multimap< ::rtl::OUString, FilterGroup::iterator >   FilterGroupEntryReferrer;

    //--------------------------------------------------------------------
    // a struct which adds helps remembering a reference to a class entry
    struct ReferToFilterClass : public ::std::unary_function< FilterName, void >
    {
    protected:
        FilterGroupEntryReferrer&   m_rClassReferrer;
        FilterGroup::iterator   m_aClassPos;

    public:
        ReferToFilterClass( FilterGroupEntryReferrer& _rClassReferrer, const FilterGroup::iterator& _rClassPos )
            :m_rClassReferrer( _rClassReferrer )
            ,m_aClassPos( _rClassPos )
        {
        }

        // operate on a single filter name
        void operator() ( const FilterName& _rName )
        {
            m_rClassReferrer.insert( FilterGroupEntryReferrer::value_type( _rName, m_aClassPos ) );
        }
    };

    //--------------------------------------------------------------------
    struct InitGlobalClasses : public ::std::unary_function< FilterClass, void >
    {
    protected:
        FilterGroup&            m_rGlobalClassGroup;
        FilterGroupEntryReferrer&   m_rClassReferrer;

    public:
        InitGlobalClasses( FilterGroup& _rGlobalClassGroup, FilterGroupEntryReferrer& _rClassReferrer )
            :m_rGlobalClassGroup( _rGlobalClassGroup )
            ,m_rClassReferrer   ( _rClassReferrer )
        {
        }

        // operate on a single class
        void operator() ( const FilterClass& _rClass )
        {
            // create an empty filter descriptor for the class
            FilterDescriptor aClassEntry;
            // set it's name (which is all we know by now)
            aClassEntry.First = _rClass.sDisplayName;

            // add it to the group
            m_rGlobalClassGroup.push_back( aClassEntry );
            // the position of the newly added class
            FilterGroup::iterator aClassEntryPos = m_rGlobalClassGroup.end();
            --aClassEntryPos;

            // and for all the sub filters of the global class, rember the class
            // (respectively the position of the class it the group for the global classes)
            ::std::for_each(
                _rClass.aSubFilters.getConstArray(),
                _rClass.aSubFilters.getConstArray() + _rClass.aSubFilters.getLength(),
                ReferToFilterClass( m_rClassReferrer, aClassEntryPos )
            );
        }
    };

    //--------------------------------------------------------------------
    static sal_Unicode s_cWildcardSeparator( ';' );
    // a helper struct which adds a fixed (Sfx-)filter to a filter group entry given by iterator
    struct ExtendDescriptorWithWildcard : public ::std::unary_function< FilterGroupEntryReferrer::value_type, void >
    {
    protected:
        String m_sWildCard;

    public:
        ExtendDescriptorWithWildcard( const String& _rWildCard )
            :m_sWildCard( &s_cWildcardSeparator, 1 )
        {
            DBG_ASSERT( _rWildCard.Len(),
                "ExtendDescriptorWithWildcard::ExtendDescriptorWithWildcard: invalid wildcard!" );
            DBG_ASSERT( _rWildCard.GetBuffer()[0] != s_cWildcardSeparator,
                "ExtendDescriptorWithWildcard::ExtendDescriptorWithWildcard: wildcard already separated!" );

            m_sWildCard += _rWildCard;
        }

        // operate on a single class entry
        void operator() ( const FilterGroupEntryReferrer::value_type& _rClassReference )
        {
            // simply add our wildcards
            _rClassReference.second->Second += m_sWildCard;
            // TODO: a more sophisticated approach: check for double wildcards
        }
    };

    //--------------------------------------------------------------------
    void lcl_GroupAndClassify(
        SfxFilterMatcherIter& _rFilterMatcher, GroupedFilterList& _rAllFilters )
    {
        _rAllFilters.swap( GroupedFilterList() );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // read the classification of filters
        FilterClassList aGlobalClasses, aLocalClasses;
        lcl_ReadClassification( aGlobalClasses, aLocalClasses );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // for the global filter classes we need an extra group in our "all filters" container
        _rAllFilters.push_front( FilterGroup() );
        FilterGroup& rGlobalFilters = _rAllFilters.front();
            // it's important to work on the reference: we want to access the members of this filter group
            // by an iterator (FilterGroup::const_iterator)
        // the referrer for the global classes
        FilterGroupEntryReferrer aGlobalClassesRef;

        // initialize the group
        ::std::for_each(
            aGlobalClasses.begin(),
            aGlobalClasses.end(),
            InitGlobalClasses( rGlobalFilters, aGlobalClassesRef )
        );
            // now we have:
            // in rGlobalFilters: a list of FilterDescriptor's, where each's discriptor's display name is set to the name of a class
            // in aGlobalClassesRef: a mapping from logical filter names to positions within rGlobalFilters
            //  this way, if we encounter an arbitrary filter, we can easily (and efficient) check if it belongs to a global class
            //  and modify the descriptor for this class accordingly

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // now add the filters
        // the group which we currently work with
        GroupedFilterList::iterator aCurrentGroup = _rAllFilters.end(); // no current group
        // the filter container of the current group - if this changes between two filters, a new group is reached
        const SfxFilterContainer*   pCurrentGroupsContainer = NULL;

        // loop through all the filters
        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter; pFilter = _rFilterMatcher.Next() )
        {
            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // check for a change in the group
            const SfxFilterContainer* pContainer = pFilter->GetFilterContainer();
            if ( pContainer != pCurrentGroupsContainer )
            {   // we reached a new group
                // -> insert a new entry in our overall-list
                aCurrentGroup = _rAllFilters.insert( _rAllFilters.end() );
                // remember the container to properly detect the next group
                pCurrentGroupsContainer = pContainer;
            }

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // add the filter to it's group
            DBG_ASSERT( aCurrentGroup != _rAllFilters.end(), "sfx2::lcl_GroupAndClassify: invalid current group!" );
            String sFilterWildcard = pFilter->GetWildcard().GetWildCard();
            aCurrentGroup->push_back( FilterDescriptor( pFilter->GetUIName(), sFilterWildcard ) );

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // check if the filter is part of a global group
            ::std::pair< FilterGroupEntryReferrer::iterator, FilterGroupEntryReferrer::iterator >
                aBelongsTo = aGlobalClassesRef.equal_range( pFilter->GetName() );
            // add the filter to the entries for these classes
            // (if they exist - if not, the range is empty and the for_each is a no-op)
            ::std::for_each(
                aBelongsTo.first,
                aBelongsTo.second,
                ExtendDescriptorWithWildcard( sFilterWildcard )
            );

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // check if the filter is part of a local filter
            // TODO
        }
    }

    //--------------------------------------------------------------------
    struct AppendFilter : public ::std::unary_function< FilterDescriptor, void >
    {
    protected:
        Reference< XFilterManager >         m_xFilterManager;
    public:
        AppendFilter( const Reference< XFilterManager >& _rxFilterManager )
            :m_xFilterManager       ( _rxFilterManager )
        {
            DBG_ASSERT( m_xFilterManager.is(), "AppendFilter::AppendFilter: invalid filter manager!" );
        }

        // operate on a single filter
        void operator() ( const FilterDescriptor& _rFilterEntry )
        {
            m_xFilterManager->appendFilter( _rFilterEntry.First, _rFilterEntry.Second );
        }
    };

    //--------------------------------------------------------------------
    struct AppendFilterGroup : public ::std::unary_function< FilterGroup, void >
    {
    protected:
        Reference< XFilterManager >         m_xFilterManager;
        Reference< XFilterGroupManager >    m_xFilterGroupManager;

    public:
        AppendFilterGroup( const Reference< XFilterManager >& _rxFilterManager )
            :m_xFilterManager       ( _rxFilterManager )
            ,m_xFilterGroupManager  ( _rxFilterManager, UNO_QUERY )
        {
        }

        // operate on a single filter group
        void operator() ( const FilterGroup& _rGroup )
        {
            try
            {
                if ( m_xFilterGroupManager.is() )
                {   // the file dialog implementation supports visual grouping of filters
                    // create a representation of the group which is understandable by the XFilterGroupManager
                    Sequence< StringPair > aFilters( _rGroup.size() );
                    ::std::copy(
                        _rGroup.begin(),
                        _rGroup.end(),
                        aFilters.getArray()
                    );
                    m_xFilterGroupManager->appendFilterGroup( ::rtl::OUString(), aFilters );
                }
                else
                {
                    ::std::for_each(
                        _rGroup.begin(),
                        _rGroup.end(),
                        AppendFilter( m_xFilterManager )
                    );
                }
            }
            catch( const Exception& )
            {
                DBG_ERROR( "AppendFilterGroup::operator(): caught an exception while adding filters!" );
            }
        }
    };

    //--------------------------------------------------------------------
    void lcl_EnsureAllFilesEntry( SfxFilterMatcherIter& _rFilterMatcher, const Reference< XFilterManager >& _rxFilterManager, ::rtl::OUString& _rFirstNonEmpty )
    {
        ::rtl::OUString sUIName;
        String          sAllFilterName( SfxResId( STR_FILTERNAME_ALL ) );
        sal_Bool        bHasAll = sal_False;

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // check if there's already a filter <ALL>
        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter && !bHasAll; pFilter = _rFilterMatcher.Next() )
        {
            if ( pFilter->GetUIName() == sAllFilterName )
                bHasAll = sal_True;
        }

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // no? -> add it
        if ( !bHasAll )
        {
            try
            {
                _rxFilterManager->appendFilter( sAllFilterName, DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
                _rFirstNonEmpty = sAllFilterName;
            }
            catch( const IllegalArgumentException& )
            {
#ifdef DBG_UTIL
                ByteString aMsg( "sfx2::lcl_EnsureAllFilesEntry: could not append Filter" );
                aMsg += ByteString( String( sAllFilterName ), RTL_TEXTENCODING_UTF8 );
                DBG_ERROR( aMsg.GetBuffer() );
#endif
            }
        }

    }

    //--------------------------------------------------------------------
    void appendFilters( SfxFilterMatcherIter& _rFilterMatcher, const Reference< XFilterManager >& _rxFilterManager, ::rtl::OUString& _rFirstNonEmpty )
    {
        DBG_ASSERT( _rxFilterManager.is(), "sfx2::appendFilters: invalid manager!" );
        if ( !_rxFilterManager.is() )
            return;

        ::rtl::OUString sUIName;

        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter; pFilter = _rFilterMatcher.Next() )
        {
            sUIName = pFilter->GetUIName();
            try
            {
                _rxFilterManager->appendFilter( sUIName, pFilter->GetWildcard().GetWildCard() );
                if ( !_rFirstNonEmpty.getLength() )
                    _rFirstNonEmpty = sUIName;

            }
            catch( IllegalArgumentException )
            {
    #ifdef DBG_UTIL
                ByteString aMsg( "Could not append Filter" );
                aMsg += ByteString( String( sUIName ), osl_getThreadTextEncoding() );
                DBG_ERRORFILE( aMsg.GetBuffer() );
    #endif
            }
        }
    }

    //--------------------------------------------------------------------
    void appendFiltersForOpen( SfxFilterMatcherIter& _rFilterMatcher, const Reference< XFilterManager >& _rxFilterManager, ::rtl::OUString& _rFirstNonEmpty )
    {
        DBG_ASSERT( _rxFilterManager.is(), "sfx2::appendFiltersForOpen: invalid manager!" );
        if ( !_rxFilterManager.is() )
            return;

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // ensure that there's an entry "all" (with wildcard *.*)
        lcl_EnsureAllFilesEntry( _rFilterMatcher, _rxFilterManager, _rFirstNonEmpty );

#ifdef DISABLE_GROUPING_AND_CLASSIFYING
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        appendFilters( _rFilterMatcher, _rxFilterManager, _rFirstNonEmpty );
#else
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // group and classify the filters
        GroupedFilterList aAllFilters;
        lcl_GroupAndClassify( _rFilterMatcher, aAllFilters );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        ::std::for_each(
            aAllFilters.begin(),
            aAllFilters.end(),
            AppendFilterGroup( _rxFilterManager )
        );
#endif
    }

//........................................................................
}   // namespace sfx2
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/10/01 16:32:30  fs
 *  initial checkin - helpers for grouping and classifying filters in the file open dialog
 *
 *
 *  Revision 1.0 01.10.01 10:28:28  fs
 ************************************************************************/

