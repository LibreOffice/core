/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SPIRIT_SUPPLEMENTS_HXX
#define INCLUDED_SPIRIT_SUPPLEMENTS_HXX

# ifndef USE_MODERN_SPIRIT
# include <boost/spirit.hpp>

namespace boost { namespace spirit
{
    template <>
    class assign_actor< std::pair<const char*,const char*> >
    {
    public:
        explicit assign_actor(std::pair<const char*,const char*>& ref_)
            : ref(ref_) {}

        template <typename T2>
        void operator()(T2 const& val) const
        { ref = val; }

        template <typename IteratorT>
        void operator()(IteratorT const& f, IteratorT const& l) const
        {
            ref.first = f, ref.second = l;
        }

    private:
        std::pair<const char*,const char*>& ref;
    };

    template<typename Target, typename Value> struct assigner
    {
        assigner( Target& rTarget, Value aValue ) :
            mrTarget(rTarget), maValue(aValue)
        {}

        void assign() const { mrTarget=maValue; }

        void operator()() const { assign(); }
        template<typename T1> void operator()(T1) const { assign(); }
        template<typename T1,typename T2> void operator()(T1,T2) const { assign(); }
        template<typename T1,typename T2,typename T3> void operator()(T1,T2,T3) const { assign(); }

        Target&     mrTarget;
        const Value maValue;
    };

    template<typename Target, typename Value> inline assigner<Target,Value>
    assign_a( Target& rTarget, Value aValue )
    {
        return assigner<Target,Value>(rTarget,aValue);
    }

    template <typename Target> inline assign_actor<Target>
    assign_a(Target& rTarget)
    {
        return assign_actor<Target>(rTarget);
    }

    template<typename Target, typename Value> struct back_pusher
    {
        back_pusher( Target& rTarget, const Value& rValue ) :
            mrTarget(rTarget), mrValue(rValue)
        {}

        void push_back() const { mrTarget.push_back(mrValue); }

        void operator()() const { push_back(); }
        template<typename T1> void operator()(T1) const { push_back(); }
        template<typename T1,typename T2> void operator()(T1,T2) const { push_back(); }
        template<typename T1,typename T2,typename T3> void operator()(T1,T2,T3) const { push_back(); }

        Target&      mrTarget;
        const Value& mrValue;
    };

    template<typename Target, typename Value> inline back_pusher<Target,Value>
    push_back_a( Target& rTarget, const Value& rValue )
    {
        return back_pusher<Target,Value>(rTarget,rValue);
    }

    template<typename Target> struct value_back_pusher
    {
        explicit value_back_pusher( Target& rTarget ) :
            mrTarget(rTarget)
        {}
        template<typename T1> void operator()(T1 val) const { mrTarget.push_back(val); }

        Target&      mrTarget;
    };

    template<typename Target> inline value_back_pusher<Target>
    push_back_a( Target& rTarget )
    {
        return value_back_pusher<Target>(rTarget);
    }
} }

# endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
