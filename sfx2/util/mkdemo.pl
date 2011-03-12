binmode stdin;
binmode stdout;
while(<>)
{
    s/t(lxsayshere)/\x0\1/;
    print $_;
}
