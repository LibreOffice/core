This is the old test implemantation of rtl::XString.
If you want to write new or better tests:
Identify the test function in the source and remark it.

The best way to remark old tests, go to the end of the source code and remark
only the test_rtl_<X>String_<function>(hRtlTestResult); so the test will not
executed any longer.

There are already some new tests for rtl::XString, take a look into the
directory sal/qa/rtl/ostring or sal/qa/rtl/oustring, where are some
replacements added.


Any other Questions?

Do not hesitate to contact me at 'lla<at>openoffice.org'.

best regards,
Lars

