#include <stdexcept>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _CMDLINE_HXX_
#include "cmdline.hxx"
#endif

//---------------------------------
/** Simple command line abstraction
*/

//################################
// Creation
//################################


CommandLine::CommandLine(size_t argc, char* argv[], const std::string& ArgPrefix) :
    m_argc(argc),
    m_argv(argv),
    m_argprefix(ArgPrefix)
{
}


//################################
// Query
//################################


/** Return the argument count
*/
size_t CommandLine::get_arg_count() const
{
    return m_argc;
}

/** Return an argument by index
    This method doesn't skip argument
    names if any, so if the second
    argument is an argument name the
    function nevertheless returns it.

    @precond    0 <= Index < GetArgumentCount

    @throws std::out_of_range exception
    if the given index is to high
*/
std::string CommandLine::get_arg(size_t Index) const
{
    OSL_PRECOND(Index < m_argc, "Index out of range");

    if (Index > (m_argc - 1))
        throw std::out_of_range("Invalid index");

    return m_argv[Index];
}


/** Returns all argument name found in the
    command line. An argument will be identified
    by a specified prefix. The standard prefix
    is '-'.
    If the are no argument names the returned
    container is empty.
*/
StringListPtr_t CommandLine::get_arg_names() const
{
    StringListPtr_t arg_cont(new StringList_t());

    for (size_t i = 0; i < m_argc; i++)
    {
        std::string argn = m_argv[i];

        if (is_arg_name(argn))
            arg_cont->push_back(argn);
    }

    return arg_cont;
}

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
std::string CommandLine::get_arg(const std::string& ArgumentName) const
{
    std::string arg_value;
    size_t i;
    for ( i = 0; i < m_argc; i++)
    {
        std::string arg = m_argv[i];

        if (ArgumentName == arg && ((i+1) < m_argc) && !is_arg_name(m_argv[i+1]))
        {
            arg_value = m_argv[i+1];
            break;
        }
    }

    if (i == m_argc)
        throw std::invalid_argument("Invalid argument name");

    return arg_value;
}


//################################
// Command
//################################


/** Set the prefix used to identify arguments in
    the command line.

    @precond    prefix is not empty

    @throws std::invalid_argument exception if
    the prefix is empty
*/
void CommandLine::set_arg_prefix(const std::string& Prefix)
{
    OSL_PRECOND(Prefix.length(), "Empty argument prefix!");

    if (0 == Prefix.length())
        throw std::invalid_argument("Empty argument prefix not allowed");

    m_argprefix = Prefix;
}


/** Returns whether a given argument is an argument name
*/
bool CommandLine::is_arg_name(const std::string& Argument) const
{
    return (0 == Argument.compare(0, m_argprefix.length(), m_argprefix));
}
