## Tile states

A tile is a piece of the rendered document. The browser asks for tiles,
coolwsd asks the kit to render them, and the result is sent back and
kept in a cache.

A tile can be in one of these states.

### Not tracked
The tile is not in the cache and no one is rendering it. The first
request moves it to **requested**.

### Requested
A request was sent to the kit. The tile is in the "being rendered" map
on the wsd side and in the kit's queue. If the same tile is asked for
again with a newer wireId, the kit's queue keeps only the newer one.

### Delivered (cached)
The kit replied with the tile data. It is now in the cache and was
sent to all subscribed sessions. The "being rendered" entry is removed.

### Invalidated
The kit sent `invalidatetiles:` for an area that covers this tile.
The cached data is marked as not valid but stays in the cache. The
next request for this tile will go to the kit again. The "being
rendered" map is *not* touched by invalidation.

### Stale
A "being rendered" entry is older than `COMMAND_TIMEOUT_MS` (5 s).
Usually the tile is low priority (a preview, another part, or an area
outside the visible one) and the kit is busy with more important tiles,
so the reply is slow. Nothing has been re-sent yet.

### Reissued
The sweep picked the stale entry, reset its start time and sent the
request to the kit again. This whole step is gated by
`ENABLE_STALE_TILE_REISSUE` and is off by default.

### Abandoned
The entry has been reissued too many times. It is removed from the
"being rendered" map. If the client still wants the tile, it must ask
again, which creates a fresh entry. Only happens when reissuing is on.

### Dropped
All sessions that wanted this tile are gone. The sweep removes the
entry without reissuing.

## Transitions

```
        request                       kit reply
not tracked ──► requested ────────────────────► delivered (cached)
                    │                                │
                    │ no reply in 5 s                │ invalidatetiles
                    ▼                                ▼
                  stale                         invalidated
                    │                                │
       ┌────────────┼────────────┐                   │ next request
       │            │            │                   ▼
   subscribers   reissue      too many          requested
   all gone      count++      reissues
       │            │            │
       ▼            ▼            ▼
    dropped     reissued     abandoned
                    │
                    │ becomes stale again
                    ▼
                  stale
```

## When transitions happen

The wsd document loop calls the stale sweep every `COMMAND_TIMEOUT_MS`.
It always drops entries with no live subscribers.

The rest is gated by `ENABLE_STALE_TILE_REISSUE` and is off by default.
When on, for the remaining stale entries, oldest first, up to
`MAX_STALE_REISSUE_PER_SWEEP`: it abandons the entry if the reissue count
is at the limit, otherwise re-sends it to the kit, bumps the count and
resets the start time.

A new client request for the same tile slot at a newer wireId resets
the reissue count, because earlier reissues were tracking older
content.

## Constants

See `wsd/TileCache.cpp` and `common/Common.hpp`.

|        Name                   |            Meaning                                                        |
|-------------------------------|---------------------------------------------------------------------------|
| `COMMAND_TIMEOUT_MS`          | A "being rendered" entry older than this is stale. Also the sweep period. |
| `MAX_STALE_REISSUE_PER_SWEEP` | Most tiles reissued in one sweep.                                         |
| `MAX_STALE_REISSUE_TOTAL`     | Reissues for one tile before it is abandoned (~40 s of futile retries).   |
| `STALE_REISSUE_WARN_AT`       | Reissue count at which the log line is promoted from INF to WRN.          |
| `ENABLE_STALE_TILE_REISSUE`   | Off by default. Re-sends swept stale tiles to the kit. In `wsd/TileCache.hpp`. |

`TRACE_MULTIPLIER` (in `common/Common.hpp`) scales `COMMAND_TIMEOUT_MS` under valgrind or coverage builds.
