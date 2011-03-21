#include <stdio.h>
/*
 * De-coupled to allow pagein to be re-used in the unx
 * splash / quick-starter
 */
extern int pagein_execute (int argc, char **argv);

int main (int argc, char **argv)
{
    return pagein_execute (argc, argv);
}

