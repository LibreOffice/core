/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configpath.hxx,v $
 * $Revision: 1.17 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#define CONFIGMGR_CONFIGPATH_HXX_

#include <rtl/ustring.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif // INCLUDED_VECTOR

namespace configmgr
{
    namespace configuration
    {
        //--------------------------------------------------------------------

        /** check if this is a well-formed name for a
            config Node (excluding set elements)
        */
        bool isSimpleName(rtl::OUString const& sName) SAL_THROW(());

        /** make a name out of <var>sName</var>,
            validating that it can be used for a config Node (excluding set elements)
            or template name.
            @throws InvalidName
                if the name is not valid for that purpose
        */
        rtl::OUString validateNodeName(rtl::OUString const& sName);

        /** make a name out of <var>sName</var>
            validating that it can be used for a config set element
            @throws InvalidName
                if the name is not valid for that purpose
        */
        rtl::OUString validateElementName(rtl::OUString const& sName);
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
        namespace Path
        {
        //------------------------------------------------------------------------

            class Component
            {
                /// holds the contents of this path component
                rtl::OUString m_aName;
            public:
                /// construct a path component from a string, without any validation
                Component(rtl::OUString const& _sName) SAL_THROW(());

                /// is this component an empty name ?
                bool isEmpty()    const SAL_THROW(()) { return m_aName.getLength() == 0; }
                /// is this component a simple name ?
                bool isSimpleName()    const SAL_THROW(());
                /// get the inner name for this component
                rtl::OUString getName()         const SAL_THROW(());
                /// get the embedded type name for this component (if any)
                rtl::OUString getTypeName()     const SAL_THROW(());

                /// get the contents of this as string (unparsed).
                rtl::OUString toPathString() const SAL_THROW(()) { return m_aName; }

                // hashing - for hash maps. compatible to equiv or matches
                size_t hashCode() const SAL_THROW(())
                { return this->getName().hashCode(); }

                /// get the contents of this as a name (unparsed). Use with care !
                rtl::OUString const& getInternalName() const SAL_THROW(()) { return m_aName; }

            };

        //-------------------------------------------------------------------------

            /// compare taking type wildcards into account
            bool matches(Component const& lhs,Component const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool before(Component const& lhs,Component const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool equiv(Component const& lhs,Component const& rhs) SAL_THROW(());
        //-------------------------------------------------------------------------

            /// construct a empty path component
            Component makeEmptyComponent() SAL_THROW(());

        //-------------------------------------------------------------------------
            /// construct a path component from a name, validating it as simple name
            Component wrapSimpleName(rtl::OUString const& _aName);

            /// construct a path component from a type and element name, using a wildcard if no type is available
            Component makeCompositeName(rtl::OUString const& _aElementName, rtl::OUString const& _aTypeName);

    //-----------------------------------------------------------------------------
            /// construct a composite path component from a element name or string, using a wildcard type
            template <class NameRep>
            Component wrapElementName(NameRep const& _aElementName) SAL_THROW(())
            {
                return makeCompositeName(_aElementName, NameRep());
            }

        //-------------------------------------------------------------------------
            /// construct a path component from an arbitrary na,e or string
            template <class NameRep>
            Component wrapSafeName(NameRep const& _aName) SAL_THROW(())
            {
                return isSimpleName(_aName) ? wrapSimpleName(_aName) : wrapElementName(_aName);
            }

    //-----------------------------------------------------------------------------
            /** lower-level representation of a path within the configuration
                <p>Keeps the data in a vector of names in reverse order! </P>
            */
            class Rep
            {
            public:
                /// construct an empty path
                Rep() SAL_THROW(()) : m_aComponents() {}

                /// construct a path consisting of a single component <var>_aName</var>
                explicit Rep(Component const& _aName) SAL_THROW(()) : m_aComponents(1,_aName) {}

                /// construct a path consisting of a path subrange
                explicit Rep(std::vector<Component>::const_reverse_iterator const& _first, std::vector<Component>::const_reverse_iterator const& _last)
                : m_aComponents(_last.base(), _first.base()) {}

                /// swap contents with another instance
                void swap(Rep& _aOther) SAL_THROW(()) { m_aComponents.swap(_aOther.m_aComponents); }

                /// modify a path by prepending <var>aName</var>
                void prepend(Component const& _aName) SAL_THROW(()) { m_aComponents.push_back(_aName); }

                /// modify a path by prepending <var>aName</var>
                void prepend(Rep const& _aOther) SAL_THROW(());

                /// get the local name (the last component of this path)
                Component const& getLocalName() const { check_not_empty(); return m_aComponents.front(); }

                /// get the next name (the first component of this path)
                Component const& getFirstName() const { check_not_empty(); return m_aComponents.back(); }

                /// set this to the remainder after the first name (drop the first component of this path)
                void dropFirstName() { check_not_empty(); m_aComponents.pop_back(); }

                /// get a /-separated string representation of this
                rtl::OUString toString(bool _bAbsolute) const SAL_THROW(());

            public:
                /// check if this is an empty path
                bool isEmpty() const SAL_THROW(()) { return m_aComponents.empty(); }

                /// Count the components of this
                std::vector<Component>::size_type countComponents() const SAL_THROW(()) { return m_aComponents.size(); }

                /// Insert a component into this path
                void insertComponent(std::vector<Component>::reverse_iterator _it, Component _aName)
                { m_aComponents.insert(_it.base(),_aName); }

                /// Remove a component from this path
                void removeComponent(std::vector<Component>::reverse_iterator _it) { m_aComponents.erase(_it.base()); }

                /// Remove all components from this path
                void clearComponents() SAL_THROW(()) { m_aComponents.clear(); }

                /// get a STL style iterator to the first component
                std::vector<Component>::const_reverse_iterator begin() const SAL_THROW(()) { return m_aComponents.rbegin(); }
                /// get a STL style iterator to after the last component
                std::vector<Component>::const_reverse_iterator end()   const SAL_THROW(()) { return m_aComponents.rend(); }

                /// get a STL style iterator to the first component
                std::vector<Component>::reverse_iterator begin_mutate() SAL_THROW(()) { return m_aComponents.rbegin(); }
                /// get a STL style iterator to after the last component
                std::vector<Component>::reverse_iterator end_mutate() SAL_THROW(())   { return m_aComponents.rend(); }

                // hashing - for hash maps
                size_t hashCode() const SAL_THROW(());

                /// preflight check for operations that require a non-empty path
                void check_not_empty() const;

            private:
                std::vector<Component> m_aComponents;
            };
        //------------------------------------------------------------------------

            /// compare taking type wildcards into account
            bool matches(Rep const& lhs,Rep const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool before(Rep const& lhs,Rep const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool equiv(Rep const& lhs,Rep const& rhs) SAL_THROW(());
        //------------------------------------------------------------------------

            /// check a path for a prefix
            bool hasMatchingPrefix(Rep const& _aPath,Rep const& _aPrefix) SAL_THROW(());

            /// remove a prefix from a path. Throws InvalidName if it isn't a prefix
            Rep stripMatchingPrefix(Rep const& _aPath,Rep const& _aPrefix);
        //------------------------------------------------------------------------

            /// distinguishes which kind of path is present in a string
            bool isAbsolutePath(rtl::OUString const& _sPath);
        //------------------------------------------------------------------------
        }
    //------------------------------------------------------------------------

        class RelativePath
        {
            Path::Rep m_aRep;
        public:
        // Construction
            /// construct a relative path from <var>aString</var> throwing InvalidName for parse errors
            static  RelativePath parse(rtl::OUString const& _aString);

            /// construct an empty relative path
            RelativePath() SAL_THROW(()) : m_aRep() { init(); }

            /// construct a relative path having <var>aRep</var> as representation
            explicit RelativePath(Path::Rep const& _aRep)
            : m_aRep(_aRep) { init(); }

            /// CONVERSION: construct a relative path having <var>aName</var> as single component
            RelativePath(Path::Component const& _aName) SAL_THROW(());

            /// build the Path that results from appending <var>aPath</var> to this
            RelativePath compose(RelativePath const& _aPath) const SAL_THROW(());

            /// check if this is an empty path
            bool isEmpty() const SAL_THROW(()) { return m_aRep.isEmpty(); }

            /// Count the components of this
            std::vector<Path::Component>::size_type getDepth() const SAL_THROW(()) { return m_aRep.countComponents(); }

            /// get the local name (the last component of this path)
            Path::Component const& getLocalName() const { return m_aRep.getLocalName(); }

            /// get the local name (the first component of this path)
            Path::Component const& getFirstName() const { return m_aRep.getFirstName(); }

            /// set this to the remainder of this path after the first name (drop the first component of this path)
            void dropFirstName() { m_aRep.dropFirstName(); }

            /// get a /-separated string representation of this
            rtl::OUString toString() const SAL_THROW(());
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            std::vector<Path::Component>::const_reverse_iterator begin() const SAL_THROW(()) { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            std::vector<Path::Component>::const_reverse_iterator end()   const SAL_THROW(()) { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            std::vector<Path::Component>::reverse_iterator begin_mutate() SAL_THROW(()) { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            std::vector<Path::Component>::reverse_iterator end_mutate() SAL_THROW(())   { return m_aRep.end_mutate(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            Path::Rep const& rep() const SAL_THROW(()) { return m_aRep; }

        private:
            void init();
        };

        /// compare taking type wildcards into account
        inline bool matches(RelativePath const& lhs,RelativePath const& rhs) SAL_THROW(())
        { return Path::matches(lhs.rep(),rhs.rep()); }

    //------------------------------------------------------------------------

        class AbsolutePath
        {
            Path::Rep m_aRep;
        public:
        // Construction
            /// construct a absolute path from <var>aString</var> throwing InvalidName for parse errors
            static AbsolutePath parse(rtl::OUString const& _aString);

            /// construct a absolute path to a whole module (toplevel) without error checking
            static AbsolutePath makeModulePath(rtl::OUString const& _aString) SAL_THROW(());

            /// construct an absolute path to the (virtual) hierarchy root
            static  AbsolutePath root() SAL_THROW(());

            /// construct an (otherwise invalid) substitute path for the root of a free-floating node
            static  AbsolutePath detachedRoot() SAL_THROW(());

            /// construct a absolute path having <var>aRep</var> as representation
            explicit AbsolutePath(Path::Rep const& _aRep) SAL_THROW(())
            : m_aRep(_aRep) { init(); }

            /// build the absolute path that results from appending <var>aPath</var> to this
            AbsolutePath compose(RelativePath const& _aPath) const SAL_THROW(());

            /// build the absolute path that results from removing the last component of this
            AbsolutePath getParentPath() const;

            /// check if this is the path to the (imaginary) root node
            bool isRoot() const SAL_THROW(()) { return m_aRep.isEmpty(); }
#if OSL_DEBUG_LEVEL > 0
            /// check if this is a path to a detached node
            bool isDetached() const SAL_THROW(());
#endif
            /// get the local name (the last component of this path)
            Path::Component const& getLocalName() const { return m_aRep.getLocalName(); }

            rtl::OUString const & getModuleName() const { return m_aRep.getFirstName().getInternalName(); }

            /// get a /-separated string representation of this
            rtl::OUString toString() const SAL_THROW(());

            /// Count the components of this
            std::vector<Path::Component>::size_type getDepth() const SAL_THROW(()) { return m_aRep.countComponents(); }
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            std::vector<Path::Component>::const_reverse_iterator begin() const SAL_THROW(()) { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            std::vector<Path::Component>::const_reverse_iterator end()   const SAL_THROW(()) { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            std::vector<Path::Component>::reverse_iterator begin_mutate() SAL_THROW(()) { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            std::vector<Path::Component>::reverse_iterator end_mutate() SAL_THROW(())   { return m_aRep.end_mutate(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            Path::Rep const& rep() const SAL_THROW(()) { return m_aRep; }
        private:
            void init() SAL_THROW(());
        };

        /// compare taking type wildcards into account
        inline bool matches(AbsolutePath const& lhs,AbsolutePath const& rhs) SAL_THROW(())
        { return Path::matches(lhs.rep(),rhs.rep()); }

        namespace Path
        {
    //------------------------------------------------------------------------
            template <class PathClass>
            bool hasPrefix(PathClass const& _aPath, PathClass const& _aPrefix) SAL_THROW(())
            {
                return hasMatchingPrefix(_aPath.rep(),_aPrefix.rep() );
            }
    //------------------------------------------------------------------------

            template <class PathClass>
            RelativePath stripPrefix(PathClass const& _aPath, PathClass const& _aPrefix)
            {
                return RelativePath( stripMatchingPrefix(_aPath.rep(),_aPrefix.rep()) );
            }
    //------------------------------------------------------------------------

    // STL Helpers
    //------------------------------------------------------------------------

            /// a weak strict ordering considering only the name part
            struct Before
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return before(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return before(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return before(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return before(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------

            /// an equality relation considering only the name part (compatible to Before)
            struct Equiv
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return equiv(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return equiv(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return equiv(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return equiv(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------

            /// a hash generator (compatible to Equiv and Before)
            struct Hash
            {
                size_t operator()(Component const& _aObject) const SAL_THROW(())
                { return _aObject.hashCode(); }
                size_t operator()(Rep const& _aObject) const SAL_THROW(())
                { return _aObject.hashCode(); }
                size_t operator()(AbsolutePath const& _aObject) const SAL_THROW(())
                { return _aObject.rep().hashCode(); }
                size_t operator()(RelativePath const& _aObject) const SAL_THROW(())
                { return _aObject.rep().hashCode(); }
            };
    //------------------------------------------------------------------------
            /// a binary predicate that is not (!) an equivalence relation

            struct Matches
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return matches(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return matches(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return matches(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return matches(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------
        }
    //------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNAME_HXX_
