/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _CMDLINE_HXX_
#define _CMDLINE_HXX_

#include "defs.hxx"

//---------------------------------
/** Simple command line abstraction
*/

class CommandLine
{
public:

    // Creation


    CommandLine(size_t argc, char* argv[]);


    // Query


    /** Returns an argument by name. If there are
        duplicate argument names in the command line,
        the first one wins.
        Argument name an the argument value must be separated
        by spaces. If the argument value starts with an
        argument prefix use quotes else the return value is
        an empty string because the value will be interpreted
        as the next argument name.
        If an argument value contains spaces use quotes.

        @precond    GetArgumentNames() -> has element ArgumentName

        @throws std::invalid_argument exception
        if the specified argument could not be
        found
    */
    std::string get_arg(const std::string& ArgumentName) const;


private:

    /** Returns whether a given argument is an argument name
    */
    bool is_arg_name(const std::string& Argument) const;

private:
    size_t      m_argc;
    char**      m_argv;

// prevent copy and assignment
private:
    CommandLine(const CommandLine&);
    CommandLine& operator=(const CommandLine&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
