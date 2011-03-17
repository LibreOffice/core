eval 'exec perl -wS $0 ${1+\"$@\"}'
    if 0;

my $sMySQLConnector = $ARGV[0];
if (! -e $sMySQLConnector)
{
    exit 1;
}
exit 0;
