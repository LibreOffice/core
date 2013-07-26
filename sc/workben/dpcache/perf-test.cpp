/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>

#include <boost/noncopyable.hpp>

using namespace std;

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~stack_printer()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

typedef std::vector<int> values_type;
typedef std::vector<size_t> indices_type;

#if 1
size_t val_count = 6000000;
double multiplier = 300000.0;
bool dump_values = false;
#else
size_t val_count = 20;
double multiplier = 10.0;
bool dump_values = true;
#endif

struct field : boost::noncopyable
{
    values_type items;   /// unique values
    indices_type data;   /// original value series as indices into unique values.
    indices_type order;  /// ascending order of the values as indices.
};

long compare(int left, int right)
{
    if (left == right)
        return 0;
    if (left < right)
        return -1;
    return 1;
}

bool has_item(const values_type& items, const indices_type& order, int val, long& index)
{
    index = items.size();
    bool found = false;
    long low = 0;
    long high = items.size() - 1;
    long comp_res;
    while (low <= high)
    {
        long this_index = (low + high) / 2;
        comp_res = compare(items[order[this_index]], val);
        if (comp_res < 0)
            low = this_index + 1;
        else
        {
            high = this_index - 1;
            if (comp_res == 0)
            {
                found = true;
                low = this_index;
            }
        }
    }
    index = low;
    return found;
}

bool check_items(const values_type& items)
{
    if (items.empty())
        return false;

    // Items are supposed to be all unique values.
    values_type copied(items);
    sort(copied.begin(), copied.end());
    copied.erase(unique(copied.begin(), copied.end()), copied.end());
    return copied.size() == items.size();
}

bool check_order(const values_type& items, const indices_type& order)
{
    // Ensure that the order is truly in ascending order.
    if (items.size() != order.size())
        return false;

    if (items.empty())
        return false;

    indices_type::const_iterator it = order.begin();
    values_type::value_type prev = items[*it];
    for (++it; it != order.end(); ++it)
    {
        values_type::value_type val = items[*it];
        if (prev >= val)
            return false;

        prev = val;
    }

    return true;
}

bool check_data(const values_type& items, const indices_type& data, const values_type& original)
{
    if (items.empty() || data.empty() || original.empty())
        return false;

    if (data.size() != original.size())
        return false;

    size_t n = data.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (items[data[i]] != original[i])
            return false;
    }
    return true;
}

bool dump_and_check(const field& fld, const values_type& original, bool dump_values)
{
    cout << "unique item count:   " << fld.items.size() << endl;
    cout << "original data count: " << fld.data.size() << endl;

    if (dump_values)
    {
        cout << "--- items" << endl;
        copy(fld.items.begin(), fld.items.end(), ostream_iterator<int>(cout, "\n"));
        cout << "--- sorted items" << endl;
        {
            indices_type::const_iterator it = fld.order.begin(), it_end = fld.order.end();
            for (; it != it_end; ++it)
            {
                cout << fld.items[*it] << endl;
            }
        }
    }

    if (!check_items(fld.items))
    {
        cout << "item check failed" << endl;
        return false;
    }

    if (!check_order(fld.items, fld.order))
    {
        cout << "order check failed" << endl;
        return false;
    }

    if (!check_data(fld.items, fld.data, original))
    {
        cout << "data check failed" << endl;
        return false;
    }

    return true;
}

void run1(const values_type& vals, bool dump_values)
{
    field fld;
    {
        stack_printer __stack_printer__("::run1 (existing algorithm)");
        values_type::const_iterator it = vals.begin(), it_end = vals.end();
        for (; it != it_end; ++it)
        {
            long index = 0;
            if (!has_item(fld.items, fld.order, *it, index))
            {
                // This item doesn't exist in the dimension array yet.
                fld.items.push_back(*it);
                fld.order.insert(
                    fld.order.begin()+index, fld.items.size()-1);
                fld.data.push_back(fld.items.size()-1);
            }
            else
                fld.data.push_back(fld.order[index]);
        }
    }

    bool res = dump_and_check(fld, vals, dump_values);
    cout << "check: " << (res ? "success" : "failure") << endl;
}

struct bucket
{
    int value;
    size_t order_index;
    size_t data_index;

    bucket(int _value, size_t _order_index, size_t _data_index) :
        value(_value), order_index(_order_index), data_index(_data_index) {}

    bucket(const bucket& r) :
        value(r.value), order_index(r.order_index), data_index(r.data_index) {}
};

void print_buckets(const vector<bucket>& buckets, const char* msg)
{
    cout << "--- buckets content (" << msg << ")" << endl;
    vector<bucket>::const_iterator it = buckets.begin(), it_end = buckets.end();
    for (; it != it_end; ++it)
    {
        cout << "value: " << it->value << "  order index: " << it->order_index
             << "  data index: " << it->data_index << endl;
    }
    cout << "---" << endl;
}

struct less_by_value : std::binary_function<bucket, bucket, bool>
{
    bool operator() (const bucket& left, const bucket& right) const
    {
        return left.value < right.value;
    }
};

struct less_by_data_index : std::binary_function<bucket, bucket, bool>
{
    bool operator() (const bucket& left, const bucket& right) const
    {
        return left.data_index < right.data_index;
    }
};

struct equal_by_value : std::binary_function<bucket, bucket, bool>
{
    bool operator() (const bucket& left, const bucket& right) const
    {
        return left.value == right.value;
    }
};

class push_back_value : std::unary_function<bucket, void>
{
    values_type& items;
public:
    push_back_value(values_type& _items) : items(_items) {}
    void operator() (const bucket& v)
    {
        items.push_back(v.value);
    }
};

class push_back_order_index : std::unary_function<bucket, void>
{
    indices_type& data_indices;
public:
    push_back_order_index(indices_type& _items) : data_indices(_items) {}
    void operator() (const bucket& v)
    {
        data_indices.push_back(v.order_index);
    }
};

void run2(const values_type& vals, bool dump_values)
{
    field fld;
    {
        stack_printer __stack_printer__("::run2 (alternative algorithm)");
        vector<bucket> buckets;
        buckets.reserve(vals.size());
        {
            // Push back all original values.
            values_type::const_iterator it = vals.begin(), it_end = vals.end();
            for (size_t i = 0; it != it_end; ++it, ++i)
                buckets.push_back(bucket(*it, 0, i));
        }

        if (buckets.empty())
        {
            cout << "error: empty buckets" << endl;
            return;
        }

//      print_buckets(buckets, "original");

        // Sort by the value.
        sort(buckets.begin(), buckets.end(), less_by_value());

//      print_buckets(buckets, "sorted");

        {
            // Set order index such that unique values have identical index value.
            size_t cur_index = 0;
            vector<bucket>::iterator it = buckets.begin(), it_end = buckets.end();
            int prev = it->value;
            it->order_index = cur_index;
            for (++it; it != it_end; ++it)
            {
                if (prev != it->value)
                    ++cur_index;

                it->order_index = cur_index;
                prev = it->value;
            }
        }

//      print_buckets(buckets, "sorted and indexed");

        // Re-sort the bucket this time by the data index.
        sort(buckets.begin(), buckets.end(), less_by_data_index());
//      print_buckets(buckets, "re-sort by data index");

        // Copy the order index series into the field object.
        fld.data.reserve(buckets.size());
        for_each(buckets.begin(), buckets.end(), push_back_order_index(fld.data));

        // Sort by the value again.
        sort(buckets.begin(), buckets.end(), less_by_value());

        // Unique by value.
        vector<bucket>::iterator it_unique_end =
            unique(buckets.begin(), buckets.end(), equal_by_value());

//      print_buckets(buckets, "uniqued");

        // Copy the unique values into items.
        vector<bucket>::iterator it_beg = buckets.begin();
        size_t len = distance(it_beg, it_unique_end);
        fld.items.reserve(len);
        for_each(it_beg, it_unique_end, push_back_value(fld.items));

        // The items are actually already sorted.  So, just insert a sequence
        // of integers from 0 and up.
        fld.order.reserve(len);
        for (size_t i = 0; i < len; ++i)
            fld.order.push_back(i);
    }

    bool res = dump_and_check(fld, vals, dump_values);
    cout << "check: " << (res ? "success" : "failure") << endl;
}

}

int main()
{
    values_type vals;
    vals.reserve(val_count);

    if (dump_values)
        cout << "--- original" << endl;

    for (size_t i = 0; i < val_count; ++i)
    {
        double v = rand();
        v /= RAND_MAX;
        v *= multiplier;
        values_type::value_type v2 = v;
        vals.push_back(v2);

        if (dump_values)
            cout << i << ": " << v2 << endl;
    }

    if (dump_values)
        cout << "---" << endl;

    run1(vals, dump_values);
    run2(vals, dump_values);

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
