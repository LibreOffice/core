cpnt.cxx contains a uno component that is uses to test the olebridge
within a JavaScript context.
In particular it has functions using out, in/out and sequence parameter
that have to be specifically treated in JScript because they are represented
by IDispatch objects.