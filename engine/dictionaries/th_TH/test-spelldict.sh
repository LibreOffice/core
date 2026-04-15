#!/usr/bin/env sh

# Script to test spelling suggestions of Thai Hunspell dictionary.
# It reads sample typos from stdin line by line and test Hunspell
# with each typo to see whether the correct spelling is suggested.
# In each line, where words are separated by space, the first word
# is the correct spelling, and the rests are typo samples.

is_ok="y"
while IFS= read -r line
do
    word=$(echo $line | cut -d' ' -f1)
    typos=$(echo $line | cut -s -d' ' -f2-)

    # First word should be correct
    res=$(echo $word | hunspell -d th_TH -a | grep -v "Hunspell")
    if ! echo "$res" | grep -q '^*$'
    then
        echo "FALSE NEG: $word"
        is_ok="n"
    fi

    # Typos should be wrong and $word should appear in the list
    for t in $typos
    do
        res=$(echo $t | hunspell -d th_TH -a | grep -v "Hunspell")
        if echo "$res" | grep -q '^*$'
        then
            echo "FALSE POS: $t (for $word)"
            is_ok="n"
        elif ! echo "$res" | grep -q "^&.*$word"
        then
            echo "NO SUGGEST: $t (for $word)"
            is_ok="n"
        fi
    done
done

[ "$is_ok" != "y" ] && exit 1

exit 0
