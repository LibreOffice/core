/*************************************************************************
 *
 *  $RCSfile: filtergrouping.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:59:31 $
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
#include <fcontnr.hxx>
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
#ifndef _WLDCRD_HXX //autogen
#include <tools/wldcrd.hxx>
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
#ifndef _SFX_OBJFAC_HXX
#include <docfac.hxx>
#endif

//........................................................................
namespace sfx2
{
//........................................................................

//#define DISABLE_GROUPING_AND_CLASSIFYING
    // not using the functionallity herein, yet

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

    <p>Now there are two types of filter classes: global ones and local ones. "Text documents" is a global class. As
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
    typedef ::std::list< FilterDescriptor >     FilterGroup;        // a list of single filter entries
    typedef ::std::list< FilterGroup >          GroupedFilterList;  // a list of all filters, already grouped

    /// the logical name of a filter
    typedef ::rtl::OUString                     FilterName;

    // a struct which holds references from a logical filter name to a filter group entry
    // used for quick lookup of classes (means class entries - entries representing a class)
    // which a given filter may belong to
    typedef ::std::map< ::rtl::OUString, FilterGroup::iterator >    FilterGroupEntryReferrer;

    /// a descriptor for a filter class (which in the final dialog is represented by one filter entry)
    typedef struct _tagFilterClass
    {
        ::rtl::OUString             sDisplayName;       // the display name
        Sequence< FilterName >      aSubFilters;        // the (logical) names of the filter which belong to the class
    } FilterClass;

    typedef ::std::list< FilterClass >                                  FilterClassList;
    typedef ::std::map< ::rtl::OUString, FilterClassList::iterator >    FilterClassReferrer;

    typedef ::std::vector< ::rtl::OUString >                            StringArray;

// =======================================================================
// = reading of configuration data
// =======================================================================

    //--------------------------------------------------------------------
    void lcl_ReadFilterClass( const OConfigurationNode& _rClassesNode, const ::rtl::OUString& _rLogicalClassName,
        FilterClass& /* [out] */ _rClass )
    {
        static const ::rtl::OUString sDisplaNameNodeName( RTL_CONSTASCII_USTRINGPARAM( "DisplayName" ) );
        static const ::rtl::OUString sSubFiltersNodeName( RTL_CONSTASCII_USTRINGPARAM( "Filters" ) );

            // the description node for the current class
        OConfigurationNode aClassDesc = _rClassesNode.openNode( _rLogicalClassName );

        // the values
        aClassDesc.getNodeValue( sDisplaNameNodeName ) >>= _rClass.sDisplayName;
        aClassDesc.getNodeValue( sSubFiltersNodeName ) >>= _rClass.aSubFilters;
    }

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
    void lcl_ReadGlobalFilters( const OConfigurationNode& _rFilterClassification, FilterClassList& _rGlobalClasses, StringArray& _rGlobalClassNames )
    {
        _rGlobalClasses.clear();
        _rGlobalClassNames.clear();

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // get the list describing the order of all global classes
        Sequence< ::rtl::OUString > aGlobalClasses;
        _rFilterClassification.getNodeValue( DEFINE_CONST_OUSTRING( "GlobalFilters/Order" ) ) >>= aGlobalClasses;

        const ::rtl::OUString* pNames = aGlobalClasses.getConstArray();
        const ::rtl::OUString* pNamesEnd = pNames + aGlobalClasses.getLength();

        // copy the logical names
        _rGlobalClassNames.resize( aGlobalClasses.getLength() );
        ::std::copy( pNames, pNamesEnd, _rGlobalClassNames.begin() );

        // Global classes are presented in an own group, so their order matters (while the order of the
        // "local classes" doesn't).
        // That's why we can't simply add the global classes to _rGlobalClasses using the order in which they
        // are returned from the configuration - it is completely undefined, and we need a _defined_ order.
        FilterClassReferrer aClassReferrer;
        ::std::for_each(
            pNames,
            pNamesEnd,
            CreateEmptyClassRememberPos( _rGlobalClasses, aClassReferrer )
        );
            // now _rGlobalClasses contains a dummy entry for each global class,
            // while aClassReferrer maps from the logical name of the class to the position within _rGlobalClasses where
            // it's dummy entry resides

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // go for all the single class entries
        OConfigurationNode aFilterClassesNode =
            _rFilterClassification.openNode( DEFINE_CONST_OUSTRING( "GlobalFilters/Classes" ) );
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
        OConfigurationNode aFilterClassesNode =
            _rFilterClassification.openNode( DEFINE_CONST_OUSTRING( "LocalFilters/Classes" ) );
        Sequence< ::rtl::OUString > aFilterClasses = aFilterClassesNode.getNodeNames();

        ::std::for_each(
            aFilterClasses.getConstArray(),
            aFilterClasses.getConstArray() + aFilterClasses.getLength(),
            ReadLocalFilter( aFilterClassesNode, _rLocalClasses )
        );
    }

    //--------------------------------------------------------------------
    void lcl_ReadClassification( FilterClassList& _rGlobalClasses, StringArray& _rGlobalClassNames, FilterClassList& _rLocalClasses )
    {
        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // open our config node
        OConfigurationTreeRoot aFilterClassification = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(),
            DEFINE_CONST_OUSTRING( "org.openoffice.Office.UI/FilterClassification" ),
            -1,
            OConfigurationTreeRoot::CM_READONLY
        );

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // go for the global classes
        lcl_ReadGlobalFilters( aFilterClassification, _rGlobalClasses, _rGlobalClassNames );

        //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        // fo for the local classes
        lcl_ReadLocalFilters( aFilterClassification, _rLocalClasses );

    }

// =======================================================================
// = grouping and classifying
// =======================================================================

    //--------------------------------------------------------------------
    // a struct which adds helps remembering a reference to a class entry
    struct ReferToFilterEntry : public ::std::unary_function< FilterName, void >
    {
    protected:
        FilterGroupEntryReferrer&   m_rEntryReferrer;
        FilterGroup::iterator       m_aClassPos;

    public:
        ReferToFilterEntry( FilterGroupEntryReferrer& _rEntryReferrer, const FilterGroup::iterator& _rClassPos )
            :m_rEntryReferrer( _rEntryReferrer )
            ,m_aClassPos( _rClassPos )
        {
        }

        // operate on a single filter name
        void operator() ( const FilterName& _rName )
        {
#ifdef DBG_UTIL
            ::std::pair< FilterGroupEntryReferrer::iterator, bool > aInsertRes =
#endif
            m_rEntryReferrer.insert( FilterGroupEntryReferrer::value_type( _rName, m_aClassPos ) );
            DBG_ASSERT( aInsertRes.second, "ReferToFilterEntry::operator(): already have an element for this name!" );
        }
    };

    //--------------------------------------------------------------------
    struct FillClassGroup : public ::std::unary_function< FilterClass, void >
    {
    protected:
        FilterGroup&                m_rClassGroup;
        FilterGroupEntryReferrer&   m_rClassReferrer;

    public:
        FillClassGroup( FilterGroup& _rClassGroup, FilterGroupEntryReferrer& _rClassReferrer )
            :m_rClassGroup      ( _rClassGroup )
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
            m_rClassGroup.push_back( aClassEntry );
            // the position of the newly added class
            FilterGroup::iterator aClassEntryPos = m_rClassGroup.end();
            --aClassEntryPos;

            // and for all the sub filters of the class, remember the class
            // (respectively the position of the class it the group)
            ::std::for_each(
                _rClass.aSubFilters.getConstArray(),
                _rClass.aSubFilters.getConstArray() + _rClass.aSubFilters.getLength(),
                ReferToFilterEntry( m_rClassReferrer, aClassEntryPos )
            );
        }
    };

    //--------------------------------------------------------------------
    static const sal_Unicode s_cWildcardSeparator( ';' );

    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    const ::rtl::OUString& getSeparatorString()
    {
        static ::rtl::OUString s_sSeparatorString( &s_cWildcardSeparator, 1 );
        return s_sSeparatorString;
    }

    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    struct CheckAppendSingleWildcard : public ::std::unary_function< ::rtl::OUString, void >
    {
        ::rtl::OUString& _rToBeExtended;

        CheckAppendSingleWildcard( ::rtl::OUString& _rBase ) : _rToBeExtended( _rBase ) { }

        void operator() ( const ::rtl::OUString& _rWC )
        {
            // check for double wildcards
            sal_Int32 nExistentPos = _rToBeExtended.indexOf( _rWC );
            if  ( -1 < nExistentPos )
            {   // found this wildcard (already part of _rToBeExtended)
                const sal_Unicode* pBuffer = _rToBeExtended.getStr();
                if  (   ( 0 == nExistentPos )
                    ||  ( s_cWildcardSeparator == pBuffer[ nExistentPos - 1 ] )
                    )
                {   // the wildcard really starts at this position (it starts at pos 0 or the previous character is a separator
                    sal_Int32 nExistentWCEnd = nExistentPos + _rWC.getLength();
                    if  (   ( _rToBeExtended.getLength() == nExistentWCEnd )
                        ||  ( s_cWildcardSeparator == pBuffer[ nExistentWCEnd ] )
                        )
                    {   // it's really the complete wildcard we found
                        // (not something like _rWC beeing "*.t" and _rToBeExtended containing "*.txt")
                        // -> outta here
                        return;
                    }
                }
            }

            if ( _rToBeExtended.getLength() )
                _rToBeExtended += getSeparatorString();
            _rToBeExtended += _rWC;
        }
    };

    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    // a helper struct which adds a fixed (Sfx-)filter to a filter group entry given by iterator
    struct AppendWildcardToDescriptor : public ::std::unary_function< FilterGroupEntryReferrer::value_type, void >
    {
    protected:
        ::std::vector< ::rtl::OUString > aWildCards;

    public:
        AppendWildcardToDescriptor( const String& _rWildCard );

        // operate on a single class entry
        void operator() ( const FilterGroupEntryReferrer::value_type& _rClassReference )
        {
            // simply add our wildcards
            ::std::for_each(
                aWildCards.begin(),
                aWildCards.end(),
                CheckAppendSingleWildcard( _rClassReference.second->Second )
            );
        }
    };

    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    AppendWildcardToDescriptor::AppendWildcardToDescriptor( const String& _rWildCard )
    {
        DBG_ASSERT( _rWildCard.Len(),
            "AppendWildcardToDescriptor::AppendWildcardToDescriptor: invalid wildcard!" );
        DBG_ASSERT( _rWildCard.GetBuffer()[0] != s_cWildcardSeparator,
            "AppendWildcardToDescriptor::AppendWildcardToDescriptor: wildcard already separated!" );

        aWildCards.reserve( _rWildCard.GetTokenCount( s_cWildcardSeparator ) );

        const sal_Unicode* pTokenLoop = _rWildCard.GetBuffer();
        const sal_Unicode* pTokenLoopEnd = pTokenLoop + _rWildCard.Len();
        const sal_Unicode* pTokenStart = pTokenLoop;
        for ( ; pTokenLoop != pTokenLoopEnd; ++pTokenLoop )
        {
            if ( ( s_cWildcardSeparator == *pTokenLoop ) && ( pTokenLoop > pTokenStart ) )
            {   // found a new token separator (and a non-empty token)
                aWildCards.push_back( ::rtl::OUString( pTokenStart, pTokenLoop - pTokenStart ) );

                // search the start of the next token
                while ( ( pTokenStart != pTokenLoopEnd ) && ( *pTokenStart != s_cWildcardSeparator ) )
                    ++pTokenStart;

                if ( pTokenStart == pTokenLoopEnd )
                    // reached the end
                    break;

                ++pTokenStart;
                pTokenLoop = pTokenStart;
            }
        }
        if ( pTokenLoop > pTokenStart )
            // the last one ....
            aWildCards.push_back( ::rtl::OUString( pTokenStart, pTokenLoop - pTokenStart ) );
    }

    //--------------------------------------------------------------------
    void lcl_InitGlobalClasses( GroupedFilterList& _rAllFilters, const FilterClassList& _rGlobalClasses, FilterGroupEntryReferrer& _rGlobalClassesRef )
    {
        // we need an extra group in our "all filters" container
        _rAllFilters.push_front( FilterGroup() );
        FilterGroup& rGlobalFilters = _rAllFilters.front();
            // it's important to work on the reference: we want to access the members of this filter group
            // by an iterator (FilterGroup::const_iterator)
        // the referrer for the global classes

        // initialize the group
        ::std::for_each(
            _rGlobalClasses.begin(),
            _rGlobalClasses.end(),
            FillClassGroup( rGlobalFilters, _rGlobalClassesRef )
        );
            // now we have:
            // in rGlobalFilters: a list of FilterDescriptor's, where each's discriptor's display name is set to the name of a class
            // in aGlobalClassesRef: a mapping from logical filter names to positions within rGlobalFilters
            //  this way, if we encounter an arbitrary filter, we can easily (and efficient) check if it belongs to a global class
            //  and modify the descriptor for this class accordingly
    }

    //--------------------------------------------------------------------
    typedef ::std::vector< ::std::pair< FilterGroupEntryReferrer::data_type, FilterGroup::iterator > >
            MapGroupEntry2GroupEntry;
            // this is not really a map - it's just called this way because it is used as a map

    struct FindGroupEntry : public ::std::unary_function< MapGroupEntry2GroupEntry::value_type, sal_Bool >
    {
        FilterGroupEntryReferrer::data_type aLookingFor;
        FindGroupEntry( FilterGroupEntryReferrer::data_type _rLookingFor ) : aLookingFor( _rLookingFor ) { }

        sal_Bool operator() ( const MapGroupEntry2GroupEntry::value_type& _rMapEntry )
        {
            return _rMapEntry.first == aLookingFor ? sal_True : sal_False;
        }
    };

    struct CopyGroupEntryContent : public ::std::unary_function< MapGroupEntry2GroupEntry::value_type, void >
    {
        void operator() ( const MapGroupEntry2GroupEntry::value_type& _rMapEntry )
        {
#ifdef DBG_UTIL
            FilterDescriptor aHaveALook = *_rMapEntry.first;
#endif
            *_rMapEntry.second = *_rMapEntry.first;
        }
    };

    //--------------------------------------------------------------------
    struct CopyNonEmptyFilter : public ::std::unary_function< FilterDescriptor, void >
    {
        FilterGroup& rTarget;
        CopyNonEmptyFilter( FilterGroup& _rTarget ) :rTarget( _rTarget ) { }

        void operator() ( const FilterDescriptor& _rFilter )
        {
            if ( _rFilter.Second.getLength() )
                rTarget.push_back( _rFilter );
        }
    };

    //--------------------------------------------------------------------
    void lcl_GroupAndClassify( SfxFilterMatcherIter& _rFilterMatcher, GroupedFilterList& _rAllFilters )
    {
        _rAllFilters.clear();

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // read the classification of filters
        FilterClassList aGlobalClasses, aLocalClasses;
        StringArray aGlobalClassNames;
        lcl_ReadClassification( aGlobalClasses, aGlobalClassNames, aLocalClasses );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // for the global filter classes
        FilterGroupEntryReferrer aGlobalClassesRef;
        lcl_InitGlobalClasses( _rAllFilters, aGlobalClasses, aGlobalClassesRef );

        // insert as much placeholders (FilterGroup's) into _rAllFilter for groups as we have global classes
        // (this assumes that both numbers are the same, which, speaking strictly, must not hold - but it does, as we know ...)
        sal_Int32 nGlobalClasses = aGlobalClasses.size();
        while ( nGlobalClasses-- )
            _rAllFilters.push_back( );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // for the local classes:
        // if n filters belong to a local class, they do not appear in their respective group explicitly, instead
        // and entry for the class is added to the group and the extensions of the filters are collected under
        // this entry
        FilterGroupEntryReferrer aLocalClassesRef;
        FilterGroup aCollectedLocals;
        ::std::for_each(
            aLocalClasses.begin(),
            aLocalClasses.end(),
            FillClassGroup( aCollectedLocals, aLocalClassesRef )
        );
        // to map from the position within aCollectedLocals to positions within the real groups
        // (where they finally belong to)
        MapGroupEntry2GroupEntry    aLocalFinalPositions;

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // now add the filters
        // the group which we currently work with
        GroupedFilterList::iterator aCurrentGroup = _rAllFilters.end(); // no current group
        // the filter container of the current group - if this changes between two filters, a new group is reached
        String aCurrentServiceName;

        String sFilterWildcard;
        ::rtl::OUString sFilterName;
        // loop through all the filters
        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter; pFilter = _rFilterMatcher.Next() )
        {
            sFilterName = pFilter->GetFilterName();
            sFilterWildcard = pFilter->GetWildcard().GetWildCard();
            AppendWildcardToDescriptor aExtendWildcard( sFilterWildcard );

            DBG_ASSERT( sFilterWildcard.Len(), "sfx2::lcl_GroupAndClassify: invalid wildcard of this filter!" );

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // check for a change in the group
            String aServiceName = pFilter->GetServiceName();
            if ( aServiceName != aCurrentServiceName )
            {   // we reached a new group

                ::rtl::OUString sDocServName = aServiceName;

                // look for the place in _rAllFilters where this ne group belongs - this is determined
                // by the order of classes in aGlobalClassNames
                GroupedFilterList::iterator aGroupPos = _rAllFilters.begin();
                DBG_ASSERT( aGroupPos != _rAllFilters.end(),
                    "sfx2::lcl_GroupAndClassify: invalid all-filters array here!" );
                    // the loop below will work on invalid objects else ...
                ++aGroupPos;
                const ::rtl::OUString* pGlobalClassNames = aGlobalClassNames.begin();
                while   (   ( aGroupPos != _rAllFilters.end() )
                        // #100545# ++pGlobalClassNames may exceed aGlobalClassNames.end()
                        &&  ( pGlobalClassNames != aGlobalClassNames.end() )
                        &&  ( *pGlobalClassNames != sDocServName )
                        )
                {
                    ++pGlobalClassNames;
                    ++aGroupPos;
                }
                if ( aGroupPos != _rAllFilters.end() )
                    // we found a global class name which matchies the doc service name -> fill the filters of this
                    // group in the respective prepared group
                    aCurrentGroup = aGroupPos;
                else
                    // insert a new entry in our overall-list
                    aCurrentGroup = _rAllFilters.insert( _rAllFilters.end() );

                // remember the container to properly detect the next group
                aCurrentServiceName = aServiceName;
            }

            DBG_ASSERT( aCurrentGroup != _rAllFilters.end(), "sfx2::lcl_GroupAndClassify: invalid current group!" );

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // check if the filter is part of a global group
            ::std::pair< FilterGroupEntryReferrer::iterator, FilterGroupEntryReferrer::iterator >
                aBelongsTo = aGlobalClassesRef.equal_range( sFilterName );
            // add the filter to the entries for these classes
            // (if they exist - if not, the range is empty and the for_each is a no-op)
            ::std::for_each(
                aBelongsTo.first,
                aBelongsTo.second,
                aExtendWildcard
            );

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
            // add the filter to it's group

            // for this, check if the filter is part of a local filter
            FilterGroupEntryReferrer::iterator aBelongsToLocal = aLocalClassesRef.find( sFilterName );
            if ( aLocalClassesRef.end() != aBelongsToLocal )
            {
#ifdef DBG_UTIL
                const ::rtl::OUString& rLocalClassDisplayName = aBelongsToLocal->second->First;
                const ::rtl::OUString& rLocalClassExtension = aBelongsToLocal->second->Second;
#endif
                // okay, there is a local class which the filter belongs to
                // -> append the wildcard
                aExtendWildcard( *aBelongsToLocal );

                MapGroupEntry2GroupEntry::iterator aThisGroupFinalPos =
                    ::std::find_if( aLocalFinalPositions.begin(), aLocalFinalPositions.end(), FindGroupEntry( aBelongsToLocal->second ) );

                if ( aLocalFinalPositions.end() == aThisGroupFinalPos )
                {   // the position within aCollectedLocals has not been mapped to a final position
                    // within the "real" group (aCollectedLocals is only temporary)
                    // -> do this now (as we just encountered the first filter belonging to this local class
                    // add a new entry which is the "real" group entry
                    aCurrentGroup->push_back( FilterDescriptor( aBelongsToLocal->second->First, String() ) );
                    // the position where we inserted the entry
                    FilterGroup::iterator aInsertPos = aCurrentGroup->end();
                    --aInsertPos;
                    // remember this pos
                    aLocalFinalPositions.push_back( MapGroupEntry2GroupEntry::value_type( aBelongsToLocal->second, aInsertPos ) );
                }
            }
            else
                aCurrentGroup->push_back( FilterDescriptor( pFilter->GetUIName(), sFilterWildcard ) );
        }

        // now just complete the infos for the local groups:
        // During the above loop, they have been collected in aCollectedLocals, but this is only temporary
        // They have to be copied into their final positions (which are stored in aLocalFinalPositions)
        ::std::for_each(
            aLocalFinalPositions.begin(),
            aLocalFinalPositions.end(),
            CopyGroupEntryContent()
        );

        // and remove local groups which do not apply - e.g. have no entries due to the limited content of the
        // current SfxFilterMatcherIter

        FilterGroup& rGlobalFilters = _rAllFilters.front();
        FilterGroup aNonEmptyGlobalFilters;
        ::std::for_each(
            rGlobalFilters.begin(),
            rGlobalFilters.end(),
            CopyNonEmptyFilter( aNonEmptyGlobalFilters )
        );
        rGlobalFilters.swap( aNonEmptyGlobalFilters );
    }

    //--------------------------------------------------------------------
    struct AppendFilter : public ::std::unary_function< FilterDescriptor, void >
    {
        protected:
            Reference< XFilterManager >         m_xFilterManager;
            FileDialogHelper_Impl*              m_pFileDlgImpl;
            bool                                m_bAddExtension;

        public:
            AppendFilter( const Reference< XFilterManager >& _rxFilterManager,
                          FileDialogHelper_Impl* _pImpl, bool _bAddExtension ) :

                m_xFilterManager( _rxFilterManager ),
                m_pFileDlgImpl  ( _pImpl ),
                m_bAddExtension ( _bAddExtension )

            {
                DBG_ASSERT( m_xFilterManager.is(), "AppendFilter::AppendFilter: invalid filter manager!" );
                DBG_ASSERT( m_pFileDlgImpl, "AppendFilter::AppendFilter: invalid filedlg impl!" );
            }

            // operate on a single filter
            void operator() ( const FilterDescriptor& _rFilterEntry )
            {
                String sDisplayText = m_bAddExtension
                    ? addExtension( _rFilterEntry.First, _rFilterEntry.Second, sal_True, *m_pFileDlgImpl )
                    : _rFilterEntry.First;
                m_xFilterManager->appendFilter( sDisplayText, _rFilterEntry.Second );
            }
    };

// =======================================================================
// = handling for the "all files" entry
// =======================================================================

    //--------------------------------------------------------------------
    sal_Bool lcl_hasAllFilesFilter( SfxFilterMatcherIter& _rFilterMatcher, String& /* [out] */ _rAllFilterName )
    {
        ::rtl::OUString sUIName;
        sal_Bool        bHasAll = sal_False;
        _rAllFilterName = String( SfxResId( STR_FILTERNAME_ALL ) );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // check if there's already a filter <ALL>
        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter && !bHasAll; pFilter = _rFilterMatcher.Next() )
        {
            if ( pFilter->GetUIName() == _rAllFilterName )
                bHasAll = sal_True;
        }
        return bHasAll;
    }

    //--------------------------------------------------------------------
    void lcl_EnsureAllFilesEntry( SfxFilterMatcherIter& _rFilterMatcher, GroupedFilterList& _rFilters )
    {
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        String sAllFilterName;
        if ( !lcl_hasAllFilesFilter( _rFilterMatcher, sAllFilterName ) )
        {
            // get the first group of filters (by definition, this group contains the global classes)
            DBG_ASSERT( !_rFilters.empty(), "lcl_EnsureAllFilesEntry: invalid filter list!" );
            if ( !_rFilters.empty() )
            {
                FilterGroup& rGlobalClasses = *_rFilters.begin();
                rGlobalClasses.push_front( FilterDescriptor( sAllFilterName, DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) ) );
            }
        }
    }

#ifdef DISABLE_GROUPING_AND_CLASSIFYING
    //--------------------------------------------------------------------
    void lcl_EnsureAllFilesEntry( SfxFilterMatcherIter& _rFilterMatcher, const Reference< XFilterManager >& _rxFilterManager, ::rtl::OUString& _rFirstNonEmpty )
    {
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        String sAllFilterName;
        if ( !lcl_hasAllFilesFilter( _rFilterMatcher, sAllFilterName ) )
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
#endif

// =======================================================================
// = filling an XFilterManager
// =======================================================================

    //--------------------------------------------------------------------
    struct AppendFilterGroup : public ::std::unary_function< FilterGroup, void >
    {
    protected:
        Reference< XFilterManager >         m_xFilterManager;
        Reference< XFilterGroupManager >    m_xFilterGroupManager;
        FileDialogHelper_Impl*              m_pFileDlgImpl;

    public:
        AppendFilterGroup( const Reference< XFilterManager >& _rxFilterManager, FileDialogHelper_Impl* _pImpl )
            :m_xFilterManager       ( _rxFilterManager )
            ,m_xFilterGroupManager  ( _rxFilterManager, UNO_QUERY )
            ,m_pFileDlgImpl         ( _pImpl )
        {
            DBG_ASSERT( m_xFilterManager.is(), "AppendFilterGroup::AppendFilterGroup: invalid filter manager!" );
            DBG_ASSERT( m_pFileDlgImpl, "AppendFilterGroup::AppendFilterGroup: invalid filedlg impl!" );
        }

        void appendGroup( const FilterGroup& _rGroup, bool _bAddExtension )
        {
            try
            {
                if ( m_xFilterGroupManager.is() )
                {   // the file dialog implementation supports visual grouping of filters
                    // create a representation of the group which is understandable by the XFilterGroupManager
                    if ( _rGroup.size() )
                    {
                        Sequence< StringPair > aFilters( _rGroup.size() );
                        ::std::copy(
                            _rGroup.begin(),
                            _rGroup.end(),
                            aFilters.getArray()
                        );
                        if ( _bAddExtension )
                        {
                            StringPair* pFilters = aFilters.getArray();
                            StringPair* pEnd = pFilters + aFilters.getLength();
                            for ( ; pFilters != pEnd; ++pFilters )
                                pFilters->First = addExtension( pFilters->First, pFilters->Second, sal_True, *m_pFileDlgImpl );
                        }
                        m_xFilterGroupManager->appendFilterGroup( ::rtl::OUString(), aFilters );
                    }
                }
                else
                {
                    ::std::for_each(
                        _rGroup.begin(),
                        _rGroup.end(),
                        AppendFilter( m_xFilterManager, m_pFileDlgImpl, _bAddExtension ) );
                }
            }
            catch( const Exception& )
            {
                DBG_ERROR( "AppendFilterGroup::operator(): caught an exception while adding filters!" );
            }
        }

        // operate on a single filter group
        void operator() ( const FilterGroup& _rGroup )
        {
            appendGroup( _rGroup, true );
        }
    };

    //--------------------------------------------------------------------
    void appendFiltersForSave( SfxFilterMatcherIter& _rFilterMatcher,
                               const Reference< XFilterManager >& _rxFilterManager,
                               ::rtl::OUString& _rFirstNonEmpty, FileDialogHelper_Impl& _rFileDlgImpl )
    {
        DBG_ASSERT( _rxFilterManager.is(), "sfx2::appendFiltersForSave: invalid manager!" );
        if ( !_rxFilterManager.is() )
            return;

        ::rtl::OUString sUIName;

        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter; pFilter = _rFilterMatcher.Next() )
        {
            ::rtl::OUString sExtension = pFilter->GetWildcard().GetWildCard();
            sUIName = addExtension( pFilter->GetUIName(), sExtension, sal_False, _rFileDlgImpl );
            try
            {
                _rxFilterManager->appendFilter( sUIName, sExtension );
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

    struct ExportFilter
    {
        ExportFilter( const rtl::OUString& _aUIName, const rtl::OUString& _aWildcard ) :
            aUIName( _aUIName ), aWildcard( _aWildcard ) {}

        rtl::OUString aUIName;
        rtl::OUString aWildcard;
    };

    //--------------------------------------------------------------------
    void appendExportFilters( SfxFilterMatcherIter& _rFilterMatcher,
                              const Reference< XFilterManager >& _rxFilterManager,
                              ::rtl::OUString& _rFirstNonEmpty, FileDialogHelper_Impl& _rFileDlgImpl )
    {
        DBG_ASSERT( _rxFilterManager.is(), "sfx2::appendExportFilters: invalid manager!" );
        if ( !_rxFilterManager.is() )
            return;

        sal_Int32                           nPDFIndex   = -1;
        sal_Int32                           nHTMLIndex  = -1;
        sal_Int32                           nFlashIndex = -1;
        sal_Int32                           nCount      = 0;
        String                              sPDFExtension = String::CreateFromAscii( "*.pdf" );
        String                              sHTMLExtension = String::CreateFromAscii( "*.htm*" );
        String                              sFlashExtension = String::CreateFromAscii( "*.swf" );
        WildCard                            aHTMLWildcardMatcher( sHTMLExtension, ';' );
        WildCard                            aPDFWildcardMatcher( sPDFExtension, ';' );
        WildCard                            aFlashWildcardMatcher( sFlashExtension, ';' );
        ::rtl::OUString                     sUIName;
        ::rtl::OUString                     sExtensions;
        std::vector< ExportFilter >         aImportantFilterGroup;
        std::vector< ExportFilter >         aFilterGroup;
        Reference< XFilterGroupManager >    xFilterGroupManager( _rxFilterManager, UNO_QUERY );

        for ( const SfxFilter* pFilter = _rFilterMatcher.First(); pFilter; pFilter = _rFilterMatcher.Next() )
        {
            sUIName     = pFilter->GetUIName();
            sExtensions = pFilter->GetWildcard().GetWildCard();

            ExportFilter aExportFilter( sUIName, sExtensions );

            String aExt = sExtensions;
            if ( nHTMLIndex == -1 && aHTMLWildcardMatcher.Matches( aExt ))
            {
                aImportantFilterGroup.insert( aImportantFilterGroup.begin(), aExportFilter );
                nHTMLIndex = 0;
                nCount++;
            }
            else if ( nPDFIndex == -1 && aPDFWildcardMatcher.Matches( aExt ))
            {
                std::vector< ExportFilter >::iterator aIter = aImportantFilterGroup.begin();
                if ( nHTMLIndex == -1 )
                    aImportantFilterGroup.insert( aIter, aExportFilter );
                else
                    aImportantFilterGroup.insert( ++aIter, aExportFilter );
                nPDFIndex = 0;
                nCount++;
            }
            else if ( nFlashIndex == -1 && aFlashWildcardMatcher.Matches( aExt ))
            {
                std::vector< ExportFilter >::iterator aIter = aImportantFilterGroup.begin();
                if ( nHTMLIndex != -1 )
                    aIter++;

                if ( nPDFIndex != -1 )
                    aIter++;

                aImportantFilterGroup.insert( aIter, aExportFilter );
                nFlashIndex = 0;
                nCount++;
            }
            else
                aFilterGroup.push_back( aExportFilter );
        }

        if ( xFilterGroupManager.is() )
        {
            // Add both html/pdf filter as a filter group to get a separator between both groups
            if ( aImportantFilterGroup.size() > 0 )
            {
                Sequence< StringPair > aFilters( aImportantFilterGroup.size() );
                for ( sal_Int32 i = 0; i < (sal_Int32)aImportantFilterGroup.size(); i++ )
                {
                    aFilters[i].First   = addExtension( aImportantFilterGroup[i].aUIName,
                                                        aImportantFilterGroup[i].aWildcard,
                                                        sal_False, _rFileDlgImpl );
                    aFilters[i].Second  = aImportantFilterGroup[i].aWildcard;
                }

                try
                {
                    xFilterGroupManager->appendFilterGroup( ::rtl::OUString(), aFilters );
                }
                catch( IllegalArgumentException )
                {
                }
            }

            if ( aFilterGroup.size() > 0 )
            {
                Sequence< StringPair > aFilters( aFilterGroup.size() );
                for ( sal_Int32 i = 0; i < (sal_Int32)aFilterGroup.size(); i++ )
                {
                    aFilters[i].First   = addExtension( aFilterGroup[i].aUIName,
                                                        aFilterGroup[i].aWildcard,
                                                        sal_False, _rFileDlgImpl );
                    aFilters[i].Second  = aFilterGroup[i].aWildcard;
                }

                try
                {
                    xFilterGroupManager->appendFilterGroup( ::rtl::OUString(), aFilters );
                }
                catch( IllegalArgumentException )
                {
                }
            }
        }
        else
        {
            // Fallback solution just add both filter groups as single filters
            for ( sal_Int32 n = 0; n < (sal_Int32)aImportantFilterGroup.size(); n++ )
            {
                try
                {
                    rtl::OUString aUIName = addExtension( aImportantFilterGroup[n].aUIName,
                                                          aImportantFilterGroup[n].aWildcard,
                                                          sal_False, _rFileDlgImpl );
                    _rxFilterManager->appendFilter( aUIName, aImportantFilterGroup[n].aWildcard  );
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

            for ( n = 0; n < (sal_Int32)aFilterGroup.size(); n++ )
            {
                try
                {
                    rtl::OUString aUIName = addExtension( aFilterGroup[n].aUIName,
                                                          aFilterGroup[n].aWildcard,
                                                          sal_False, _rFileDlgImpl );
                    _rxFilterManager->appendFilter( aUIName, aFilterGroup[n].aWildcard );
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
    }

    //--------------------------------------------------------------------
    void appendFiltersForOpen( SfxFilterMatcherIter& _rFilterMatcher,
                               const Reference< XFilterManager >& _rxFilterManager,
                               ::rtl::OUString& _rFirstNonEmpty, FileDialogHelper_Impl& _rFileDlgImpl )
    {
        DBG_ASSERT( _rxFilterManager.is(), "sfx2::appendFiltersForOpen: invalid manager!" );
        if ( !_rxFilterManager.is() )
            return;

#ifdef DISABLE_GROUPING_AND_CLASSIFYING
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // ensure that there's an entry "all" (with wildcard *.*)
        lcl_EnsureAllFilesEntry( _rFilterMatcher, _rxFilterManager, _rFirstNonEmpty );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        appendFilters( _rFilterMatcher, _rxFilterManager, _rFirstNonEmpty );
#else

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // group and classify the filters
        GroupedFilterList aAllFilters;
        lcl_GroupAndClassify( _rFilterMatcher, aAllFilters );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // ensure that we have the one "all files" entry
        lcl_EnsureAllFilesEntry( _rFilterMatcher, aAllFilters );

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // the first non-empty string - which we assume is the first overall entry
        if ( !aAllFilters.empty() )
        {
            const FilterGroup& rFirstGroup = *aAllFilters.begin();  // should be the global classes
            if ( !rFirstGroup.empty() )
                _rFirstNonEmpty = rFirstGroup.begin()->First;
            // append first group, without extension
            AppendFilterGroup aGroup( _rxFilterManager, &_rFileDlgImpl );
            aGroup.appendGroup( rFirstGroup, false );
        }

        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
        // append the filters to the manager
        if ( !aAllFilters.empty() )
        {
            ::std::list< FilterGroup >::iterator pIter = aAllFilters.begin();
            ++pIter;
            ::std::for_each(
                pIter, // first filter group was handled seperately, see above
                aAllFilters.end(),
                AppendFilterGroup( _rxFilterManager, &_rFileDlgImpl ) );
        }
#endif
    }

    ::rtl::OUString addExtension( const ::rtl::OUString& _rDisplayText,
                                  const ::rtl::OUString& _rExtension,
                                  sal_Bool _bForOpen, FileDialogHelper_Impl& _rFileDlgImpl )
    {
        static ::rtl::OUString sAllFilter( RTL_CONSTASCII_USTRINGPARAM( "(*.*)" ) );
        static ::rtl::OUString sOpenBracket( RTL_CONSTASCII_USTRINGPARAM( " (" ) );
        static ::rtl::OUString sCloseBracket( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
        ::rtl::OUString sRet = _rDisplayText;

        if ( sRet.indexOf( sAllFilter ) == -1 )
        {
            String sExt = _rExtension;
            if ( !_bForOpen )
                // show '*' in extensions only when opening a document
                sExt.EraseAllChars( '*' );
            sRet += sOpenBracket;
            sRet += sExt;
            sRet += sCloseBracket;
        }
        _rFileDlgImpl.addFilterPair( _rDisplayText, sRet );
        return sRet;
    }

//........................................................................
}   // namespace sfx2
//........................................................................


